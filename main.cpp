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

#define TOGGLE_DEBUG 6

#define NONCE_SIZE_BITS (32-TOGGLE_DEBUG)
#define FIELD_SIZE_BITS (18)

#define NUMBER_OF_SETS (1<<(NONCE_SIZE_BITS-FIELD_SIZE_BITS))

#define NONCE_TARGET 0x000fffff
#define HITS_PERCENTAGE 1.5
#define HITS_REQUIRED ((float) NONCE_TARGET/0xffffffff*(1<<FIELD_SIZE_BITS)*HITS_PERCENTAGE)

#define CALCULATE_HITS_REQUIRED(t,p) ((float) t/0xffffffff*(1<<FIELD_SIZE_BITS)*p)

#define OUTPUT_FILE_NAME "C:/Users/lucas/Documents/codeblocksproj/fastsha/output.csv"


char lookupBinToChar(const uint32_t inp){
    const char lookupTable[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    return lookupTable[inp&0x0f];
}

void writeResultToFile(std::ofstream& oFile, std::mutex& oFileMtx, const uint32_t* inp1, const uint32_t* inp2, boost::shared_ptr<std::vector<uint32_t>> lowValueFieldNumbersPtr, uint32_t numberOfSets = NUMBER_OF_SETS, double hitsPercentage = HITS_PERCENTAGE, uint32_t nonceTarget = NONCE_TARGET, uint32_t nonceSizeBits = NONCE_SIZE_BITS, uint32_t fieldSizeBits = FIELD_SIZE_BITS){
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
    oFile << "]," << numberOfSets << "," << hitsPercentage << "," << nonceTarget << "," << nonceSizeBits << "," << fieldSizeBits;
    oFile << "]" << std::endl;
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
            if((*genHash).h < NONCE_TARGET){
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
            boost::shared_ptr<vargroup> genHash = helper_full_sha256(inp1,inp2,i,preservedMessageH);
            if((*genHash).a < NONCE_TARGET){
                hits++;
            }
            i++;
        }
        if(HITS_REQUIRED < hits){
            (*results).push_back(field);
        }
        std::cout << "RUN: " << field << " HITS: " << hits << std::endl;
    }
    return results;
}

boost::shared_ptr<std::vector<std::vector<boost::shared_ptr<std::vector<uint32_t>>>>> calculateEntriesList(const uint32_t* inp1, const uint32_t* inp2, uint32_t* nonceTargetsPtr, size_t nonceTargetsPtrLen, double* hitsPercentagesPtr, size_t hitsPercentagesPtrLen){
    boost::shared_ptr<vargroup> preservedMessageH(nullptr);
    uint32_t i = 0;
    boost::shared_ptr<std::vector<std::vector<boost::shared_ptr<std::vector<uint32_t>>>>> results(new std::vector<std::vector<boost::shared_ptr<std::vector<uint32_t>>>>
                                                                                                  (nonceTargetsPtrLen,std::vector<boost::shared_ptr<std::vector<uint32_t>>>
                                                                                                   (hitsPercentagesPtrLen,nullptr)));
    for(size_t indT = 0; indT < nonceTargetsPtrLen; indT++){
        for(size_t indP = 0; indP < hitsPercentagesPtrLen; indP++){
            (*results)[indT][indP] = boost::shared_ptr<std::vector<uint32_t>>(new std::vector<uint32_t>());
        }
    }

    for(uint32_t field = 0; field < NUMBER_OF_SETS; field++){
        std::vector<uint32_t> hits(nonceTargetsPtrLen,0);
        for(uint32_t iter = 0; iter < 1<<FIELD_SIZE_BITS; iter++){
            boost::shared_ptr<vargroup> genHash = helper_full_sha256(inp1,inp2,i,preservedMessageH);
            for(size_t indT = 0; indT < nonceTargetsPtrLen; indT++){
                if((*genHash).a < nonceTargetsPtr[indT]){
                    hits[indT]++;
                }
            }
            i++;
        }
        for(size_t indT = 0; indT < nonceTargetsPtrLen; indT++){
            for(size_t indP = 0; indP < hitsPercentagesPtrLen; indP++){
                if(field%5==0)
                    std::cout << "RUN: " << field << " HITS: " << hits[indT] << " REQ: " << CALCULATE_HITS_REQUIRED(nonceTargetsPtr[indT],hitsPercentagesPtr[indP]) << std::endl;
                if(CALCULATE_HITS_REQUIRED(nonceTargetsPtr[indT],hitsPercentagesPtr[indP]) < hits[indT]){
                    (*results)[indT][indP].get()[0].push_back(field);
                }
            }
        }
    }
    return results;
}

void makeTrainingDataOnBlockList(std::ofstream& outputFile, std::mutex& outputFileMtx, const uint32_t* inp1, const uint32_t* inp2, uint32_t* nonceTargetsPtr, size_t nonceTargetsPtrLen, double* hitsPercentagesPtr, size_t hitsPercentagesPtrLen){
    boost::shared_ptr<std::vector<std::vector<boost::shared_ptr<std::vector<uint32_t>>>>> lowValueFieldsPtr = calculateEntriesList(inp1, inp2, nonceTargetsPtr, nonceTargetsPtrLen, hitsPercentagesPtr, hitsPercentagesPtrLen);
    for(size_t indT = 0; indT < nonceTargetsPtrLen; indT++){
        for(size_t indP = 0; indP < hitsPercentagesPtrLen; indP++){
            writeResultToFile(outputFile,outputFileMtx,inp1,inp2,(*lowValueFieldsPtr)[indT][indP],NUMBER_OF_SETS,hitsPercentagesPtr[indP],nonceTargetsPtr[indT],NONCE_SIZE_BITS,FIELD_SIZE_BITS);
        }
    }
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

void managerList(const DataType* inp, size_t inpLen, uint32_t* nonceTargetsPtr, size_t nonceTargetsPtrLen, double* hitsPercentagesPtr, size_t hitsPercentagesPtrLen){
    std::ofstream outputFile(OUTPUT_FILE_NAME,std::ios::out|std::ios::trunc);
    std::mutex outputFileMtx;

    std::vector<std::thread> threads;
    for(size_t i = 0; i < inpLen; i++){
        if(i%4==0){
            for(std::thread& t: threads){
                if(t.joinable()){
                    t.join();
                }
            }
        }
        std::thread t(makeTrainingDataOnBlockList,std::ref(outputFile),std::ref(outputFileMtx),inp[i].d1.data,inp[i].d2.data,nonceTargetsPtr,nonceTargetsPtrLen,hitsPercentagesPtr,hitsPercentagesPtrLen);
        threads.push_back(std::move(t));
    }

    for(std::thread& t: threads){
        if(t.joinable()){
            t.join();
        }
    }

    outputFile.flush();
    outputFile.close();
}

uint32_t lookupCharToBin(const char c1, const char c2){
    const uint32_t lookupTable[] = {
        0x00,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f,   0x00,
        0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
        0,      1,      2,      3,      4,      5,      6,      7,
        8,      9,      0,      0,      0,      0,      0,      0,
        };
    return (lookupTable[c1&0x1f]<<4)|lookupTable[c2&0x1f];
}

void populateArray(std::string inp, uint32_t* out){
    for(std::size_t i = 0; i < inp.length()>>3; i++){
        uint32_t n1 = lookupCharToBin(inp.at(i*8),  inp.at(i*8+1));
        uint32_t n2 = lookupCharToBin(inp.at(i*8+2),inp.at(i*8+3));
        uint32_t n3 = lookupCharToBin(inp.at(i*8+4),inp.at(i*8+5));
        uint32_t n4 = lookupCharToBin(inp.at(i*8+6),inp.at(i*8+7));
        uint32_t tot = (n1<<24)|(n2<<16)|(n3<<8)|n4;
        out[i] = tot;
    }
}



boost::shared_ptr<std::vector<uint32_t>> checker(std::string inp1, std::string inp2, std::size_t* positionsPtr, std::size_t positionsPtrLen, std::size_t sizeOfSet,uint32_t target){
    uint32_t parsedInp1[16];
    uint32_t parsedInp2[3];
    populateArray(inp1,parsedInp1);
    populateArray(inp2,parsedInp2);

    boost::shared_ptr<vargroup> preservedMessageH(nullptr);
    boost::shared_ptr<std::vector<uint32_t>> hitsPtr(new std::vector<uint32_t>(positionsPtrLen,0));
    for(std::size_t posi = 0; posi < positionsPtrLen; posi++){
        for(uint32_t nonce = positionsPtr[posi]*sizeOfSet; nonce < (positionsPtr[posi]+1)*sizeOfSet; nonce++){
            boost::shared_ptr<vargroup> genHash = helper_full_sha256(parsedInp1,parsedInp2,nonce,preservedMessageH);
            if((*genHash).a < target){
                hitsPtr.get()[0][posi]++;
            }
        }

    }
    return hitsPtr;
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
    //manager(heads,sizeof(heads)/sizeof(DataType));
    //manager(heads,2);
    /*
    uint32_t targets[] = {0x00ffffff,0x000fffff,0x0000ffff};
    double percentages[] = {1.3,1.5,1.7,2};
    managerList(heads,sizeof(heads)/sizeof(Data1Type),targets,sizeof(targets)/sizeof(uint32_t),percentages,sizeof(percentages)/sizeof(double));
    */

    std::size_t fie[] = {204,128,146,88,17,};
boost::shared_ptr<std::vector<uint32_t>> hit = checker("00000020cd9764d031209e1e2944a66b693f5ad3a2d7d03c879303000000000000000000f8548e980042a99ba8fdbc15ae10916f8ff35f15e6aa9a283d36c28e","f3f7972cdd3cf65f17220f17",fie,5,262144,1048575);

    for(uint32_t hi:hit.get()[0]){
        std::cout << hi << " " << CALCULATE_HITS_REQUIRED(0x000fffff,1.3) << std::endl;
    }
    std::cout << std::endl;

    return 0;
}
