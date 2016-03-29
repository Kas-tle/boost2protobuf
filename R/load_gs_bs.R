#' @rdname load_gs_bs
#' @export
#' @importClassesFrom flowWorkspace GatingSetList
#' @importFrom flowWorkspace GatingSetList
#' @useDynLib GatingSetLoader
load_gslist_bs<-function(path){
#  browser()
  path <- normalizePath(path,mustWork = TRUE)
  if(!file.exists(path))
    stop(path,"' not found!")
  dirs<-list.dirs(path,full.names = TRUE, recursive = FALSE)
#   browser()
  res <- lapply(dirs,function(this_dir){
#        browser()
        .load_gs(output = this_dir, files = list.files(this_dir))$gs      
      })
  samples <- readRDS(file.path(path,"samples.rds"))
  GatingSetList(res, samples = samples)
  
}
#' @title load a GatingSet/GatingSetList from disk.
#'
#' @description
#' It is used to load the legacy GatingSet saved in boost serialization format.
#' @param path A character scalar giving the path to load the GatingSet from.
#'
#' @return
#' \code{load_gs_bs} returns a GatingSet object
#' \code{load_gslist}_bs returns a GatingSetList object
#'
#' @examples
#' \dontrun{
#' 	gs <- load_gs_bs(path="tempFolder")
#'  # save it in pb format
#' 	save_gs(gs,path="newFolder", cdf = "link)
#' }
#' @rdname load_gs_bs
#' @export
#' @aliases load_gslist_bs
load_gs_bs<-function(path){
#  browser()
  path <- normalizePath(path,mustWork = TRUE)
  if(!file.exists(path))
    stop(path,"' not found!")
  files<-list.files(path)
#   browser()
  .load_gs(output = path, files = files)$gs
  
}
#' unserialization functions to be called by wrapper APIs
#' @importFrom tools file_ext
#' @importClassesFrom flowWorkspace GatingSet
#' @importFrom flowWorkspace flowData flowData<- isNcdf
.load_gs <- function(output,files){
  dat.file <- file.path(output,files[grep(".dat$|.txt$|.xml$",files)])
  rds.file<-file.path(output,files[grep(".rds$",files)])
  
  nc.file<-file.path(output,files[grep(".nc$|.nc.trans$",files)])
  #   browser()
  if(length(dat.file)==0)
    stop(".dat file missing in ",output)
  if(length(dat.file)>1)
    stop("multiple .dat files found in ",output)
  fileext <- file_ext(dat.file)
  
  typeID <- switch(fileext
      , "dat" = 0
      , "txt" = 1
      , "xml" = 2
      , 0
  )
  if(length(rds.file)==0)
    stop(".rds file missing in ",output)
  if(length(rds.file)>1)
    stop("multiple .rds files found in ",output)
  
  message("loading R object...")
  gs <- readRDS(rds.file)
  
  #deal with legacy archive
  if(class(gs) == "GatingSetInternal")
  {
    thisSet <- attr(gs,"set")
    thisGuid <- attr(gs,"guid")
    if(is.null(thisGuid))
      thisGuid <- .uuid_gen()
#        browser()
    thisGH <- thisSet[[1]]
    thisTree <- attr(thisGH, "tree")
    thisPath <- dirname(attr(thisGH, "dataPath"))
    thisData <- graph::nodeDataDefaults(thisTree)[["data"]]
    fs <- thisData[["data"]][["ncfs"]]
    
    axis <- sapply(thisSet, function(gh){
          thisTree <- attr(thisGH, "tree")
          thisData <- graph::nodeDataDefaults(thisTree)[["data"]]
#                            browser()
          thisSample <- attr(thisGH, "name")
          thisChnls <- colnames(fs@frames[[thisSample]])
          thisAxis <- thisData[["axis.labels"]]
          if(is.null(thisAxis))
            list()
          else{
            names(thisAxis) <- thisChnls
            thisAxis
          }
          
        }, simplify = FALSE)
    
    
    gs <- new("GatingSet", flag = TRUE, guid = thisGuid, axis = axis, data = fs)
  }
  
  if(!.hasSlot(gs, "transformation"))
    gs@transformation <- list()
  
  if(!.hasSlot(gs, "compensation"))
    gs@compensation <- NULL
  
  
  guid <- try(slot(gs,"guid"),silent=T)
  if(class(guid)=="try-error"){
    #generate the guid for the legacy archive
    gs@guid <- .uuid_gen()
  }
  
  message("loading tree object...")
  
  
  gs@pointer <- .cpp_loadGatingSet(dat.file, typeID)
  
  
  if(isNcdf(gs))
  {
    if(length(nc.file)==0)
      stop(".nc file missing in ",output)
    flowData(gs)@file <- nc.file
    
  }
  message("Done")
  list(gs=gs,files=c(dat.file,rds.file))
}
