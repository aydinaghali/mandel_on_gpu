//metal stuff
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <ctime>
#include "bmp.h"

enum ParamIndex {
	P_WIDTH = 0,
	P_HEIGHT = 1,
	P_SX = 2,
	P_EX = 3,
	P_SY = 4,
	P_EY = 5,
	P_RANGE_X = 6,
	P_RANGE_Y = 7,
	P_STEP_X = 8,
	P_STEP_Y = 9,
};

int main(){
	//clock_t tStart, tEnd;

	//init with device
	const char* SHADER_FILE_NAME = "./shader.metal";
	const char* SHADER_FUNC_NAME = "compute_shader";

	MTL::Device* device = MTL::CreateSystemDefaultDevice();
	
	std::ifstream t(SHADER_FILE_NAME);
	std::stringstream buffer;
	buffer << t.rdbuf();
	NS::String* fileContent = NS::String::alloc()->string(buffer.str().c_str(), NS::StringEncoding::ASCIIStringEncoding);
	MTL::Library* library = device->newLibrary(fileContent, MTL::CompileOptions::alloc()->init(), (NS::Error**)NULL);
	if(library == NULL){
		fprintf(stderr, "Failed to find library '%s'.\n", SHADER_FILE_NAME);
		exit(1);
	}
	
	NS::String* funcName = NS::String::alloc()->string(SHADER_FUNC_NAME, NS::StringEncoding::ASCIIStringEncoding);
	MTL::Function* shaderFunc = library->newFunction(funcName);
	if(shaderFunc == NULL){
		fprintf(stderr, "Failed to find function '%s'.\n", SHADER_FUNC_NAME);
		exit(1);
	}

	MTL::ComputePipelineState* funcMPO = device->newComputePipelineState(shaderFunc, (NS::Error**)NULL);
	if(funcMPO == NULL){
		fprintf(stderr, "Failed to create MPO.\n");
		exit(1);
	}

	MTL::CommandQueue* commandQueue = device->newCommandQueue();
	if(commandQueue == NULL){
		fprintf(stderr, "Failed to create Command queue.\n");
		exit(1);
	}

	//prepare data
	const unsigned long WIDTH = 16384;
	const unsigned long HEIGHT = 16384;
	const float SX = -1.5f;
	const float EX = 0.5f;
	const float SY = -1.0f;
	const float EY = 1.0f;
	const float range_x = EX-SX;
	const float range_y = EY-SY;
	const float step_x = range_x / (float)WIDTH;
	const float step_y = range_y / (float)HEIGHT;

	MTL::Buffer* buffParams = device->newBuffer(10, MTL::ResourceStorageModeShared);
	assert(buffParams!=NULL);
	float* paramCont = (float *)buffParams->contents();
	paramCont[0] = WIDTH;
	paramCont[1] = HEIGHT;
	paramCont[2] = SX;
	paramCont[3] = EX;
	paramCont[4] = SY;
	paramCont[5] = EY;
	paramCont[6] = range_x;
	paramCont[7] = range_y;
	paramCont[8] = step_x;
	paramCont[9] = step_y;
	
	MTL::TextureDescriptor* textureDesc = MTL::TextureDescriptor::alloc()->init();
    textureDesc->setWidth(WIDTH);
    textureDesc->setHeight(HEIGHT);
    textureDesc->setPixelFormat( MTL::PixelFormatRGBA8Unorm );
    textureDesc->setTextureType( MTL::TextureType2D );
    textureDesc->setStorageMode( MTL::StorageModeManaged );
    textureDesc->setUsage( MTL::ResourceUsageSample | MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
    MTL::Texture *texture = device->newTexture( textureDesc );
	assert(texture!=NULL);

	MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
	assert(commandBuffer!=NULL);
	MTL::ComputeCommandEncoder* commandEncoder = commandBuffer->computeCommandEncoder();
	assert(commandEncoder!=NULL);

	//encodeAddCommand
	commandEncoder->setComputePipelineState(funcMPO);
	commandEncoder->setBuffer(buffParams, 0, 0);
	commandEncoder->setTexture(texture, 1);
	MTL::Size gridSize = MTL::Size::Make(WIDTH, HEIGHT, 1);

	NS::UInteger MPOthreadGroupSize = funcMPO->maxTotalThreadsPerThreadgroup();
	MTL::Size threadGroupSize = MTL::Size::Make((MPOthreadGroupSize>(WIDTH*HEIGHT)?WIDTH*HEIGHT:MPOthreadGroupSize), 1, 1);
	printf("Starting shader...\n");
	//tStart = clock();
	commandEncoder->dispatchThreads(gridSize, threadGroupSize);
	commandEncoder->endEncoding();
	commandBuffer->commit();
	commandBuffer->waitUntilCompleted();
	//tEnd = clock();
	//printf("Done in %0.4fs.\n", (double)(tEnd-tStart)/CLOCKS_PER_SEC);
	printf("Done.\n");
	
	unsigned int* pixelBuff = (unsigned int*)malloc(WIDTH*HEIGHT*4);
	texture->getBytes(pixelBuff, WIDTH*4, MTL::Region::Make2D(0, 0, WIDTH, HEIGHT), 0);

	printf("Start writing image...\n");
	//tStart = clock();
	bmp img;
	bmp_init(&img, "img.bmp", WIDTH, HEIGHT);
	bmp_write_header(&img);
	static char pixbuff[WIDTH*HEIGHT*3];
	bmp_buff img_buff;
	bmp_buff_init(&img_buff, &img, pixbuff);
	for(unsigned int i=0; i<WIDTH*HEIGHT; ++i){
		bmp_buff_push_pixel(&img_buff, (pixelBuff[i]&0xffffff));
	}
	bmp_buff_write(&img_buff);
	bmp_free(&img);
	//tEnd = clock();
	//printf("Done in %0.4fs.\n", (double)(tEnd-tStart)/CLOCKS_PER_SEC);
	printf("Done.\n");

	//cleanups
	commandEncoder->release();
	commandBuffer->release();
	buffParams->release();	
	texture->release();
	textureDesc->release();
	fileContent->release();	
	library->release();
	funcName->release();
	shaderFunc->release();
	commandQueue->release();
	funcMPO->release();
	device->release();

	return 0;
}

