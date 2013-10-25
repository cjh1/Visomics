
sample1Range <- c(1,2,3,5)
sample2Range <- c(1,3,5,7)

input_table <- matrix(seq(1:24), 3)

filter <- function(mat, columns) {
  sample <- matrix(nrow=nrow(mat), ncol=length(columns))
  index <- 1
  for(col in columns) {
    sample[,index] <- mat[,col]
    index <- index + 1
  }

  return(sample)
}

sample1Array <- filter(input_table, sample1Range)
sample2Array <- filter(input_table, sample2Range)

ANOVA_table = matrix(nrow=0, ncol=1)
ANOVA_volcano = matrix(nrow=0, ncol=1)

errValue<-0
pValue <- sapply( seq(length=nrow(sample1Array)),
function(x) {summary(aov(sample1Array[x,] ~ sample2Array[x,]))[[1]][[1,"Pr(>F)"]] })

log2FC<-log2(rowMeans(sample1Array))-log2(rowMeans(sample2Array))
FCFun <- function(x){
if(!is.finite(x)){RerrValue <<- 2; return(-NaN)}
if (x<0) {return(-1/(2^x))} else {return(2^x)}}
foldChange<-sapply(log2FC, FCFun)

ANOVA_table
colnames(ANOVA_table) <- c("P-Value")
for (value in pValue) {
  ANOVA_table <- rbind(ANOVA_table, value)
} 

for (value in foldChange) {
  ANOVA_volcano <- rbind(ANOVA_volcano, value)
}

colnames(ANOVA_volcano) <- c("Fold Change (Sample 1 -> Sample 2)")

print(ANOVA_table)
print(ANOVA_volcano)
