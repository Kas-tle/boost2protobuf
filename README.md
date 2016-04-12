
# boost2protobuf: An tool to convert the legacy GatingSet archive to protocol buffer format     

This package facilitates `flowWorkspace` package to deal with the gated data that was archived in boost serialization format.

### INSTALLATION

```r
install.packages("devtools") 
library(devtools) #load it
install_github("boost2protobuf/RGLab", ref="master")
```

### First, convert the old archive to pb format

```r
library(boost2protobuf)
boost2protobuf(old_gs_path, new_gs_path)
```

### Then load it using flowWorkspace
```r
library(flowWorkspace)
gs <- load_gs(new_gs_path)
gh <- gs[[1]]
#plot the hierarchy tree
plot(gh)
#show all the cell populations(/nodes)
getNodes(gh)
#show the population statistics
getPopStats(gh)
#plot the gates
plotGate(gh) 
```
