class OutDataParser:
    def __init__(self, out, numberOfSets):
        self.numberOfSets = numberOfSets
        self.out = out
        self.parsedOut = self.__decompressTargetSet(out)
        
    def __decompressTargetSet(self, targets):
        ret = [[0]]*self.numberOfSets
        for target in targets:
            ret[target] = [1]
        return ret