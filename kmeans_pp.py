import numpy as np
import pandas as pd
import os
import mykmeanssp as C
import sys

def read_data(file_path):
    extension = os.path.splitext(file_path)[1]
    if extension == '.csv':
        return pd.read_csv(file_path)
    else:
        file = open(file_path)
        data = file.readlines()
        file.close()
        for i in range(len(data)):
            data[i] = data[i].split(",")
        if data[0] == '\n':
            return 0
        for x in data:
            for i in range(len(x)):
                x[i] = float(x[i])
        return pd.DataFrame(data,columns=[i for i in range(len(data[0]))])



def join_data(data1,data2):
    data = pd.merge(data1,data2, how = 'inner', on = 0)
    data.sort_values(by= 0 ,ascending= True,inplace=True)
    data.set_index(0,inplace= True)
    return data


def centroids_intializtion(data,k):
    centroids = []
    np.random.seed(1234)
    rand_index = np.random.choice(data.index)
    centroids.append(data.loc[rand_index].to_numpy().tolist())
    print(int(rand_index), end = ",")
    for s in range(k-1):
        D_x = np.zeros(len(data.index))
        for i in data.index:
            point = data.loc[i].to_numpy()
            min = float('inf')
            for centroid in centroids:
                D = np.linalg.norm(centroid-point)
                if (D < min): min = D
            D_x[int(i)] = min
        sum_D = D_x.sum()
        prob = [D_x[int(j)]/sum_D for j in data.index]
        rand_index = np.random.choice(data.index,p = np.array(prob))
        centroids.append(data.loc[rand_index].to_numpy().tolist())
        if s != k-2: print(int(rand_index),end =",")
        else: print(int(rand_index))
        
    return centroids

def main(args):
    max_iter = 300
    if len(args) == 4:
        k = int(args[0]) 
        eps = float(args[1])
        file_name1 = args[2]
        file_name2 = args[3]
    else:
        k = int(args[0]) 
        max_iter = int(args[1]) 
        eps = float(args[2]) 
        file_name1 = args[3]
        file_name2 = args[4]

    data = join_data(read_data(file_name1),read_data(file_name2))
    if k >= len(data) or k < 1:
         print("Invalid number of clusters!")
         return 0
    if max_iter > 1000 or max_iter < 1 :
         print("Invalid maximum iteration!")
         return 0
    if eps < 0 : 
        print ("Invalid epsilon!")
        return 0

    centroids = centroids_intializtion(data, k)
    data = data.to_numpy().tolist()
    d = len(centroids[0])

    final_centroids = C.fit_c(centroids, data, d, k,max_iter,eps)
    for x in final_centroids:
        print(",".join(f"{coord:.4f}" for coord in x))
    return 1

if __name__ == "__main__":

    main(sys.argv[1:])

