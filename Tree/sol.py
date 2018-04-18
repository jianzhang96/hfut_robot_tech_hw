import trees
import treePlotter

fr=open('data.txt')
dataset=[inst.strip().split('\t') for inst in fr.readlines()]
labels=['其他选择','饿否','价格','餐馆类型','餐馆人数','等待时间']
dtree=trees.createTree(dataset,labels)
treePlotter.createPlot(dtree)
