import random
import tensorflow as tf
class DataAccessor:
    def __init__(self, fname, numberOfSets=4096, hitsPercentage=1.3, nonceTarget=1048575, nonceSizeBits=32, fieldSizeBits=20):
        self.fname = fname
        self.numberOfSets = numberOfSets
        self.hitsPercentage = hitsPercentage
        self.nonceTarget = nonceTarget
        self.nonceSizeBits = nonceSizeBits
        self.fieldSizeBits = fieldSizeBits
        
    def __decompressTargetSet(self, targets):
        ret = [[0]]*self.numberOfSets
        for target in targets:
            ret[target] = [1]
        return ret
    
    def __decompressInp(self, inp):
        return [int(inp[i:i+8],16) for i in range(0, len(inp), 8)]
    
    
    def __genData(self, ignoreHitsPercentage=False, ignoreNonceTarget=False, ignoreNonceSizeBits=False, ignoreFieldSizeBits=False):
        file = open(self.fname)
        for line in file.readlines():
            try:
                elems = eval(line)
                if elems[3] == self.numberOfSets \
                and (True if ignoreHitsPercentage else elems[4] == self.hitsPercentage) \
                and (True if ignoreNonceTarget else elems[5] == self.nonceTarget) \
                and (True if ignoreNonceSizeBits else elems[6] == self.nonceSizeBits) \
                and (True if ignoreFieldSizeBits else elems[7] == self.fieldSizeBits) \
                and len(elems[0]) == 128 \
                and len(elems[1]) == 24 \
                and isinstance(elems[2], list):
                    elems[0] = self.__decompressInp(elems[0]+elems[1])
                    elems[2] = self.__decompressTargetSet(elems[2])
                    yield elems
            except GeneratorExit:
                raise GeneratorExit
            except:
                pass
         
        
        
    def genTrainingData(self, ignoreHitsPercentage=False, ignoreNonceTarget=False, ignoreNonceSizeBits=False, ignoreFieldSizeBits=False):
        data = list(self.__genData(ignoreHitsPercentage,ignoreNonceTarget,ignoreNonceSizeBits,ignoreFieldSizeBits))
        while True:
            temp = random.choice(data)
            yield (tf.constant([temp[0]],dtype="uint32"),tf.constant([temp[2]],dtype="uint32"))
            
            