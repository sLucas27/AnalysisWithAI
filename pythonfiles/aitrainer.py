import DataManagement
import tensorflow as tf
import InputParser
import outputParser
import numpy as np

class Trainer:
    def __init__(self, modelName, trainingDataName, hitsPercentage=1.3, nonceTarget=0x000fffff, nonceSizeBits=32, fieldSizeBits=20):
        self.__modelName = modelName
        self.model = tf.keras.models.load_model(self.__modelName)
        self.__dataAcessor = DataManagement.DataAccessor(trainingDataName,self.model.output_shape[1],hitsPercentage,nonceTarget,nonceSizeBits,fieldSizeBits)
        
    def compileModel(self,optimizer="adamax",loss="binary_crossentropy"):
        self.model.compile(optimizer=optimizer,loss=loss,metrics=["accuracy"])
        
    def recompileModel(self,optimizer="adamax",loss="binary_crossentropy"):
        self.compileModel(optimizer=optimizer,loss=loss)
        
    def trainModel(self,epochs=100,stepsPerEpoch=100):
        self.model.fit(x=self.__dataAcessor.genTrainingData(),epochs=epochs,steps_per_epoch=stepsPerEpoch,verbose=1)
        
    def saveModel(self,modelOutputName=None):
        if modelOutputName is None:
            modelOutputName = self.__modelName
        self.model.save(modelOutputName)
        
    def predictModel(self,inp1,inp2=None):
        InputParser.InpDataParser
        return self.model.predict(x=InputParser.InpDataParser(inp1,inp2).getParsedInput(),steps=1)
    
    def predictionToRanges(self,inp1,inp2=None,topNRanges=5):
        ttemp = self.predictModel(inp1,inp2)[0]
        temp = np.argpartition(ttemp,-topNRanges)
        res = temp[-topNRanges:]
        for i,e in enumerate(res):
            print(res[i],ttemp[i])
        genStr = "std::size_t fie[] = {"
        for i in res:
            genStr += str(i) + ","
        genStr += "};\n"
        genStr += "boost::shared_ptr<std::vector<uint32_t>> hit = checker("
        genStr += "\"" + inp1 + "\",\"" + inp2 + "\",fie"
        genStr += "," + str(len(res)) + "," + str(2**self.__dataAcessor.fieldSizeBits) + "," + str(self.__dataAcessor.nonceTarget)
        genStr += ");"
        return genStr
        
        '''
        std::size_t fie[] = {240,196,38,204,111,};
        boost::shared_ptr<std::vector<uint32_t>> hit = checker(
            "00006020ba3b06d0356b9155a0da9e756882c7873517cf787b280e00000000000000000005dbbb74dcddbe63b6c81c41b1d1d99fcd09b28b527746090a54bed8","9a157b81706bec5f17220f17",fie,5,262144,1048575
        );
            '''
        

#trainer = Trainer("newmodel","bigoutput.csv",1.3,0x000fffff,26,18)
#trainer.compileModel()
#trainer.trainModel(100,100)
trainer = Trainer("goodmodelv2","bigoutput.csv",1.3,0x000fffff,26,18)

#model.fit(x=dataAccessor.genTrainingData(),y=None,epochs=1000,steps_per_epoch=100,verbose=1)
#model.predict(x=dataAccessor.genTrainingData(),steps=1)
#trainer.predictionToRanges("0000c0200e207f3760827927f2acdc1e73d3b130639ab868b6770b000000000000000000b2c070851890259df3a16480ad1b735cd2569b05cb3dd26f5f04932a","28c0e644d65bec5f17220f17")

#"0000c0200e207f3760827927f2acdc1e73d3b130639ab868b6770b000000000000000000b2c070851890259df3a16480ad1b735cd2569b05cb3dd26f5f04932a","28c0e644d65bec5f17220f17"
#"00400020669542a8651b7b11db91545e92594bbc952a6ddd6ab60e00000000000000000049e9c0c31e5a2c12121bb63f3556b369fe3f793aec14b97831446941","683669f95158ec5f17220f17"
