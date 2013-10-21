errValue<-0
pValue <- sapply( seq(length=nrow(sample1Array)),
function(x) {summary(aov(sample1Array[x,] ~ sample2Array[x,]))[[1]][[1,"Pr(>F)"]] })
log2FC<-log2(rowMeans(sample1Array))-log2(rowMeans(sample2Array))
FCFun <- function(x){
if(!is.finite(x)){RerrValue <<- 2; return(-NaN)}
if (x<0) {return(-1/(2^x))} else {return(2^x)}}
foldChange<-sapply(log2FC, FCFun)
