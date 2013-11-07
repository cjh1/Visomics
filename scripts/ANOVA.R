sample1Range <- c(2,3)
sample2Range <- c(3,4)

#input_table <- list(c("A", "B", "C", "D"), as.double(c(1,2,3,4)), as.double(c(1,2,3,4)),as.double(c(1,2,3,4)),as.double(c(1,2,3,4)))

#write('test', stderr())

filter <- function(columnList, columnIndexes) {
  write(typeof(columnList), stderr())
  sample <- matrix(nrow=length(columnList[[1]]), ncol=0)
  for(i in columnIndexes) {
    sample <- cbind(sample,  columnList[[i]])
  }

  return(sample)
}

sample1Array <- filter(input_table, sample1Range)
sample2Array <- filter(input_table, sample2Range)

errValue<-0
pValue <- sapply( seq(length=nrow(sample1Array)),
function(x) {summary(aov(sample1Array[x,] ~ sample2Array[x,]))[[1]][[1,"Pr(>F)"]] })

log2FC<-log2(rowMeans(sample1Array))-log2(rowMeans(sample2Array))
FCFun <- function(x){
if(!is.finite(x)){RerrValue <<- 2; return(-NaN)}
if (x<0) {return(-1/(2^x))} else {return(2^x)}}
foldChange<-sapply(log2FC, FCFun)
print(pValue)
ANOVA_table <- list("lizard"=input_table[[1]], "PValues"=sample1Array)

ANOVA_volcano <- list("lizard"=input_table[[1]])#,"FFold Change (Sample 1 -> Sample 2)"=foldChange, "Test"=foldChange)


