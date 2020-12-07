/*
* Add test vectors
* Uplaod to github repository
*
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>

#include <Funcs.h>

#define TOGGLE_DEBUG 9

#define NONCE_SIZE_BITS 32-TOGGLE_DEBUG
#define FIELD_SIZE_BITS 16

#define NUMBER_OF_SETS (1<<(NONCE_SIZE_BITS-FIELD_SIZE_BITS))

#define NONCE_TARGET 0x00ffffff
#define HITS_PERCENTAGE 1.1
#define HITS_REQUIRED (float) NONCE_TARGET/0xffffffff*(1<<FIELD_SIZE_BITS)*HITS_PERCENTAGE

//ENTER OUTPUT FILE PATHE
#define OUTPUT_FILE_NAME "output.csv"


struct data1Type{
    uint32_t data[16];
};

struct data2Type{
    uint32_t data[3];
};

data1Type data1[3] = {
    {
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
        0x00000000, 0x00000000,
    },
    {
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
        0x00010203, 0x04050607,
    },
    {
        0x14499318, 0x52517638,
        0x32638230, 0x94525729,
        0x14719525, 0x27745128,
        0x03337250, 0x10369376,
        0x06040537, 0x53982738,
        0x68258214, 0x18781574,
        0x45634192, 0x53376388,
        0x84253073, 0x18253826,
    },
};

data2Type data2[3] = {
    {
        0x00000000, 0x00000000,
        0x00000000,
    },
    {
        0x00010203, 0x04050607,
        0x00010203,
    },
    {
        0x45305720, 0x02580252,
        0x58225523,
    },
};

char lookupBinToChar(uint32_t inp){
    const char lookupTable[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    return lookupTable[inp&0x0f];
}

void writeResultToFile(std::ofstream& oFile, std::mutex& oFileMtx, uint32_t* inp1, uint32_t* inp2, boost::shared_ptr<std::vector<uint32_t>> lowValueFieldNumbersPtr){
    std::lock_guard<std::mutex> oFileLck(oFileMtx);
    oFile << "[\"";
    for(uint32_t i = 0; i < 16; i++){
        for(uint32_t s = 8; s > 0; s--){
            char c = lookupBinToChar(inp1[i]>>(s-1)*4);
            oFile << c;
        }
    }
    //oFile.write(reinterpret_cast<char*>((uint8_t*) inp1),4*16);
    oFile << "\",\"";
    for(uint32_t i = 0; i < 3; i++){
        for(uint32_t s = 8; s > 0; s--){
            char c = lookupBinToChar(inp2[i]>>(s-1)*4);
            oFile << c;
        }
    }
    //oFile.write(reinterpret_cast<char*>((uint8_t*) inp2),4*3);
    oFile << "\",[";
    for(uint32_t lowValueFieldNumber:lowValueFieldNumbersPtr.get()[0]){
        oFile << lowValueFieldNumber << ",";
    }
    oFile << "]," << NUMBER_OF_SETS << "," << HITS_PERCENTAGE << "," << NONCE_TARGET << "," << NONCE_SIZE_BITS << "," << FIELD_SIZE_BITS;
    oFile << "]," << std::endl;
    oFile.flush();
}

void writeResultToFile(std::ofstream& oFile, std::mutex& oFileMtx, uint32_t* inp, boost::shared_ptr<std::vector<uint32_t>> lowValueFieldNumbersPtr){
    writeResultToFile(oFile,oFileMtx,inp,inp+16,lowValueFieldNumbersPtr);
}

uint32_t calculateEntry(uint32_t* inp1, uint32_t* inp2){
    boost::shared_ptr<vargroup> preservedMessageH(nullptr);

    uint32_t i = 0;
    uint32_t bestField = 0;
    uint32_t bestFieldHits = 0;
    for(uint32_t field = 0; field < NUMBER_OF_SETS; field++){
        uint32_t hits = 0;
        for(uint32_t iter = 0; iter < 1<<FIELD_SIZE_BITS; iter++){
            i++;
            boost::shared_ptr<vargroup> genHash = helper_full_sha256(inp1,inp2,i,preservedMessageH);
            if((*genHash).a < NONCE_TARGET){
                hits++;
            }
        }
        if(bestFieldHits < hits){
            bestField = field;
            bestFieldHits = hits;
        }
    }
    return bestField;
}

boost::shared_ptr<std::vector<uint32_t>> calculateEntries(uint32_t* inp1, uint32_t* inp2){
    boost::shared_ptr<vargroup> preservedMessageH(nullptr);
    uint32_t i = 0;
    boost::shared_ptr<std::vector<uint32_t>> results(new std::vector<uint32_t>());
    for(uint32_t field = 0; field < NUMBER_OF_SETS; field++){
        uint32_t hits = 0;
        for(uint32_t iter = 0; iter < 1<<FIELD_SIZE_BITS; iter++){
            i++;
            boost::shared_ptr<vargroup> genHash = helper_full_sha256(inp1,inp2,i,preservedMessageH);
            if((*genHash).a < NONCE_TARGET){
                hits++;
            }
        }
        if(HITS_REQUIRED < hits){
            (*results).push_back(field);
        }
        std::cout << "RUN: " << field << " HITS: " << hits << std::endl;
    }
    return results;
}

void makeTrainingDataOnBlock(std::ofstream& outputFile, std::mutex& outputFileMtx, uint32_t* inp1, uint32_t* inp2){
    boost::shared_ptr<std::vector<uint32_t>> lowValueFieldsPtr = calculateEntries(inp1, inp2);
    writeResultToFile(outputFile,outputFileMtx,inp1,inp2,lowValueFieldsPtr);
}

void manager(data1Type* inp1, data2Type* inp2, size_t inpLen){
    std::ofstream outputFile(OUTPUT_FILE_NAME,std::ios::out|std::ios::trunc);
    std::mutex outputFileMtx;

    std::vector<std::thread> threads;
    for(size_t i = 0; i < inpLen; i++){
        std::thread t(makeTrainingDataOnBlock,std::ref(outputFile),std::ref(outputFileMtx),inp1[i].data,inp2[i].data);
        threads.push_back(std::move(t));
    }

    for(std::thread& t: threads){
        t.join();
    }

    outputFile.flush();
    outputFile.close();
}

int main()
{
    std::cout << "HITS REQUIRED: " << HITS_REQUIRED << std::endl;
    std::cout << "WORK AMOUNT: " << NUMBER_OF_SETS << std::endl;
    std::cout << "WORK SIZE: " << (1<<(FIELD_SIZE_BITS)) << std::endl;

    /*boost::shared_ptr<std::vector<uint32_t>> res = calculateEntries(data1[2],data2[2]);
    for(uint32_t re:res.get()[0]){
        std::cout << re << std::endl;
    }*/
    std::cout << NONCE_TARGET << " : " << HITS_REQUIRED << std::endl;

    manager(data1,data2,2);

    return 0;
}
