import tensorflow as tf
class InpDataParser:
    def __init__(self,inp1,inp2=None):
        if len(inp1) != 128+24 and inp2 is None:
            raise Exception("input must be of length 152")
        elif len(inp1) != 128:
            raise Exception("input1 must be of length 128")
        elif len(inp2) != 24:
            raise Exception("input2 must be of length 24")
        self.inp1 = inp1
        self.inp2 = inp2
        self.parsedInp = tf.constant([self.__decompressInp(inp1+inp2)],dtype="uint32")
    
    def __decompressInp(self, inp):
        return [int(inp[i:i+8],16) for i in range(0, len(inp), 8)]
    
    def getParsedInput(self):
        return self.parsedInp