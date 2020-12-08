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
#include <Data.h>

#define TOGGLE_DEBUG 9

#define NONCE_SIZE_BITS 32-TOGGLE_DEBUG
#define FIELD_SIZE_BITS 16

#define NUMBER_OF_SETS (1<<(NONCE_SIZE_BITS-FIELD_SIZE_BITS))

#define NONCE_TARGET 0x00ffffff
#define HITS_PERCENTAGE 1.1
#define HITS_REQUIRED (float) NONCE_TARGET/0xffffffff*(1<<FIELD_SIZE_BITS)*HITS_PERCENTAGE

#define OUTPUT_FILE_NAME "C:/Users/lucas/Documents/codeblocksproj/fastsha/output.csv"


char lookupBinToChar(const uint32_t inp){
    const char lookupTable[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    return lookupTable[inp&0x0f];
}

void writeResultToFile(std::ofstream& oFile, std::mutex& oFileMtx, const uint32_t* inp1, const uint32_t* inp2, boost::shared_ptr<std::vector<uint32_t>> lowValueFieldNumbersPtr){
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

void writeResultToFile(std::ofstream& oFile, std::mutex& oFileMtx, const uint32_t* inp, boost::shared_ptr<std::vector<uint32_t>> lowValueFieldNumbersPtr){
    writeResultToFile(oFile,oFileMtx,inp,inp+16,lowValueFieldNumbersPtr);
}

uint32_t calculateEntry(const uint32_t* inp1, const uint32_t* inp2){
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

boost::shared_ptr<std::vector<uint32_t>> calculateEntries(const uint32_t* inp1, const uint32_t* inp2){
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

void makeTrainingDataOnBlock(std::ofstream& outputFile, std::mutex& outputFileMtx, const uint32_t* inp1, const uint32_t* inp2){
    boost::shared_ptr<std::vector<uint32_t>> lowValueFieldsPtr = calculateEntries(inp1, inp2);
    writeResultToFile(outputFile,outputFileMtx,inp1,inp2,lowValueFieldsPtr);
}

void manager(const DataType* inp, size_t inpLen){
    std::ofstream outputFile(OUTPUT_FILE_NAME,std::ios::out|std::ios::trunc);
    std::mutex outputFileMtx;

    std::vector<std::thread> threads;
    for(size_t i = 0; i < inpLen; i++){
        std::thread t(makeTrainingDataOnBlock,std::ref(outputFile),std::ref(outputFileMtx),inp[i].d1.data,inp[i].d2.data);
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
    std::cout << "NONCE TARGET: " << NONCE_TARGET << std::endl;

    std::cout << "------------------------------------Verifying validity with test vector------------------------------------" << std::endl;
    std::cout << "Generate output: ";
    boost::shared_ptr<vargroup> temp(nullptr);
    uint32_t dat[] = {0x01000000,0x9500c43a,0x25c62452,0x0b5100ad,0xf82cb9f9,0xda72fd24,0x47a496bc,0x600b0000,0x00000000,0x6cd86237,0x0395dedf,0x1da2841c,0xcda0fc48,0x9e3039de,0x5f1ccdde,0xf0e83499,0x1a65600e,0xa6c8cb4d,0xb3936a1a};
    temp = helper_full_sha256(dat,0xe3143991,temp);
    for(uint32_t i = 0; i < 8; i++){
        for(uint32_t s = 8; s > 0; s--){
            char c = lookupBinToChar(((uint32_t*)(temp.get()))[i]>>(s-1)*4);
            std::cout << c;
        }
    }
    std::cout << std::endl;
    std::cout << "Expected output: " << "CAC383CDF62F68EFAA8064E35F6FC4DFC8AA74610C6580ED1729000000000000" << std::endl;
    std::cout << "This is the 123456 block in the BTC blockchain." << std::endl;
    //https://learnmeabitcoin.com/explorer/block/0000000000002917ED80650C6174AAC8DFC46F5FE36480AAEF682FF6CD83C3CA
    std::cout << "https://learnmeabitcoin.com/explorer/block/0000000000002917ED80650C6174AAC8DFC46F5FE36480AAEF682FF6CD83C3CA" << std::endl;
    std::cout << "----------------------------------------------------END----------------------------------------------------" << std::endl;


    //make sure to replace siyeof(... with a low number (around 2) so that the computer used for testing is not being overwhelmed with tons of parallel threaded sha256 operations :D
    manager(heads,sizeof(heads)/sizeof(DataType));

    return 0;
}
