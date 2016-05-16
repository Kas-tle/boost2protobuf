#' convert the GatingSet from boost to protobuf format
#' @param from the old archive path
#' @param to the new archive path
#' @param ... other arguments passed to \link{save_gs}
#' @export 
#' @examples 
#' \dontrun{
#' #set cdf to 'link' to avoid the redundant copy of flow data
#' boost2protobuf(old_gs_path, new_gs_path, cdf = "link")
#' }
boost2protobuf <- function(from, to, ...){
  subdir <- list.dirs(from, recursive = FALSE)
  if(length(subdir) == 0){
    message("loading bs archive...")
    suppressMessages(gs <- load_gs(from))
    message("saving to pb archive...")
    suppressMessages(save_gs(gs, to, ...))
    message("GatingSet is now saved in pb format and can be loaded with 'load_gs'")
  }else{
    message("loading bs archive...")
    suppressMessages(gs <- load_gslist(from))
    message("saving to pb archive...")
    suppressMessages(save_gslist(gs, to, ...))
    message("GatingSetList is now saved in pb format and can be loaded with 'load_gslist'")
  }
  
}