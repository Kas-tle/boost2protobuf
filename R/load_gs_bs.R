#' @rdname load_gs
#' @importClassesFrom flowWorkspace GatingSetList
#' @importFrom flowWorkspace GatingSetList
#' @useDynLib GatingSetLoader
load_gslist<-function(path){
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
#' \code{load_gs} returns a GatingSet object
#' \code{load_gslist}_bs returns a GatingSetList object
#'
#' @examples
#' \dontrun{
#' 	gs <- GatingSetLoader:::load_gs(path="tempFolder")
#'  # save it in pb format
#' 	GatingSetLoader:::save_gs(gs,path="newFolder", cdf = "link)
#' }
#' @rdname load_gs
#' @aliases load_gslist
load_gs<-function(path){
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
  
  
  gs@pointer <- .cpp_loadGatingSet(dat.file, typeID, isPB = FALSE)
  
  
  if(isNcdf(gs))
  {
    if(length(nc.file)==0)
      stop(".nc file missing in ",output)
    flowData(gs)@file <- nc.file
    
  }
  message("Done")
  list(gs=gs,files=c(dat.file,rds.file))
}

#' @rdname save_gs
save_gslist<-function(gslist,path,...){
  
  if(!file.exists(path))
    dir.create(path = path)
  #do the dir normalization again after it is created
  path <- normalizePath(path,mustWork = TRUE)
  lapply(gslist,function(gs){
#        this_dir <- tempfile(pattern="gs",tmpdir=path)
#        dir.create(path = this_dir)
#        browser()
        guid <- gs@guid
        if(length(guid)==0){
          gs@guid <- flowWorkspace:::.uuid_gen()
          guid <- gs@guid
        }
        this_dir <- file.path(path,guid) 
        
#        invisible(flowWorkspace:::.save_gs(gs,path = this_dir, ...))
        suppressMessages(save_gs(gs,path = this_dir, ...))
      }, level =1)
#  browser()
  #save sample vector
  saveRDS(names(gslist@samples),file=file.path(path,"samples.rds"))
  message("Done\nTo reload it, use 'load_gslist' function\n")
  
  
}

#' @title save a boost GatingSet/GatingSetList as protocol buffer format
#'
#' @description
#' Save a GatingSet/GatingSetList to the disk using protocol buffer library.
#'
#' @param G A \code{GatingSet}
#' @param gslist A \code{GatingSetList}
#' @param path A character scalar giving the path to save/load the GatingSet to/from.
#' @param overwrite A logical scalar specifying whether to overwrite the existing folder.
#' @param cdf a character scalar. The valid options are :"copy","move","skip","symlink","link" specifying what to do with the cdf data file.
#'              Sometime it is more efficient to move or create a link of the existing cdf file to the archived folder.
#'              It is useful to "skip" archiving cdf file if raw data has not been changed.
#' @param ... other arguments: not used.
#'
#'
#' @return
#' \code{load_gs} returns a GatingSet object
#' \code{load_gslist} returns a GatingSetList object
#'
#' @seealso \code{\link{GatingSet-class}},\code{\link{GatingSetList-class}}
#'
#' @examples
#' \dontrun{
#' 	gs<- GatingSetLoader:::load_gs(bsFolder)
#'  GatingSetLoader:::save_gs(pbFolder)
#'
#' }
#' @rdname save_gs
#' @aliases save_gs load_gs save_gslist load_gslist
save_gs<-function(G,path,overwrite = FALSE
    , cdf = c("copy","move","skip","symlink","link")
    , ...){
#  browser()
  cdf <- match.arg(cdf)
  fileext <- 'pb'
  
  
  guid <- G@guid
  if(length(guid) == 0){
    G@guid <- .uuid_gen()
    guid <- G@guid
  }
  rds_toSave <- paste(guid,"rds",sep=".")
  dat_toSave <- paste(guid,fileext,sep=".")
  
  if(file.exists(path)){
    path <- normalizePath(path,mustWork = TRUE)
    if(overwrite){
      this_files <- list.files(path)
      #validity check for non-empty folder
      if(length(this_files)!=0)
      {
        rds_ind <- grep("\\.rds$",this_files)
        dat_ind <- grep(paste0("\\.",fileext,"$"),this_files)
        
        if(length(rds_ind)!=1||length(dat_ind)!=1){
          stop("Not a valid GatingSet archiving folder!")
        }else{
          this_rds <- this_files[rds_ind]
          this_dat <- this_files[dat_ind]
          
          if(this_rds!=rds_toSave||this_dat!=dat_toSave){
            stop("The GatingSet doesn't match the archived files in: ", path)
          }
        }
      }
      
      #validity check for cdf
      if(isNcdf(G)){
        if(length(this_files)!=0){
          cdf_ind <- grep("\\.nc$",this_files)
          if(length(cdf_ind) != 1){
            stop("Not a valid GatingSet archiving folder!")
          }
        }
        
      }
      if(length(this_files)!=0)
      {
        #start to delete the old files in path
        file.remove(file.path(path,rds_toSave))
        file.remove(file.path(path,dat_toSave))
        
        if(isNcdf(G)){
          #check if the target path is the same as current cdf path
#            browser()
          this_cdf <- file.path(path,this_files[cdf_ind])
          if(normalizePath(getData(G)@file)==this_cdf){
            cdf <- "skip"
          }
          if(cdf != "skip"){
            file.remove(this_cdf)
          }
        }
      }
      
    }else{
      stop(path,"' already exists!")
    }
    
  }else{
    dir.create(path = path)
    #do the dir normalization again after it is created
    path <- normalizePath(path,mustWork = TRUE)
    
  }
#  browser()
  invisible(.save_gs(G=G,path = path, cdf = cdf, ...))
  message("Done\nTo reload it, use 'load_gs' function\n")
  
  
}





#' serialization functions to be called by wrapper APIs
.save_gs <- function(G,path, cdf = c("copy","move","skip","symlink","link")){
  
#    browser()
  cdf <- match.arg(cdf)
  fileext <- 'pb'
  
  if(!file.exists(path))
    stop("Folder '",path, "' does not exist!")
  #generate uuid for the legacy GatingSet Object
  if(length(G@guid)==0){
    G@guid <- .uuid_gen()
  }
  guid <- G@guid
  
  rds.file<-file.path(path,paste(guid,"rds",sep="."))
  dat.file<-file.path(path,paste(guid,fileext,sep="."))
  
  
  filestoSave <- c(rds.file,dat.file)
  #save ncdf file
  if(cdf != "skip" && isNcdf(G))
  {
    from<-flowData(G)@file
#      browser()
    if(cdf == "move"){
      message("moving ncdf...")
      ncFile <- file.path(path,basename(from))
      res <- file.rename(from,ncFile)
      #reset the file path for ncdfFlowSet
      flowData(G)@file <- ncFile
    }else{
      
      ncFile<-tempfile(tmpdir=path,fileext=".nc")
      
      if(cdf == "copy"){
        message("saving ncdf...")
        res <- file.copy(from=from,to=ncFile)
      }
      else if(cdf == "symlink"){
        message("creating the symbolic link to ncdf...")
        res <- file.symlink(from=from,to=ncFile)
      }else if(cdf == "link"){
        message("creating the hard link to ncdf...")
        res <- file.link(from=from,to=ncFile)
      }
    }
    if(!res){
      stop("failed to ",cdf," ",from,"!")
    }
    filestoSave<-c(filestoSave,ncFile)
  }
  
  message("saving tree object...")
  #save external pointer object
  .cpp_saveGatingSet(G@pointer, dat.file, format = 0, isPB = TRUE)
  
  message("saving R object...")
  saveRDS(G,rds.file)
  
  filestoSave
  
}
