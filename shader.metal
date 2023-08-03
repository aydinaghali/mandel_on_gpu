#include <metal_stdlib>
using namespace metal;

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

kernel void compute_shader(device float* params [[buffer(0)]],
							texture2d<half, access::write> tex [[texture(1)]],
							uint2 index [[thread_position_in_grid]],
							uint2 gridSize [[threads_per_grid]]		){
	const uint MAX_ITER = 5000;
	const float SX = params[P_SX];
	const float STEP_X = params[P_STEP_X];
	const float SY = params[P_SY];
	const float STEP_Y = params[P_STEP_Y];
	
	float x0 = SX + index.x*STEP_X;
	float y0 = SY + index.y*STEP_Y;
	float x = 0.0;
	float y = 0.0;
	uint iter = 0;
	
	while(((x*x+y*y)<=4) && iter<MAX_ITER){
		float xtemp = x*x - y*y + x0;
		y = 2*x*y + y0;
		x = xtemp;
		++iter;
	}
	
	half color = iter<MAX_ITER ? (0.5 + 0.5 * cos(3.0 + iter * 0.15)) : 0.0; //iter<MAX_ITER ? 1.0 : 0.0;
	tex.write(half4(color, color, color, 1.0), index, 0);	

	//half color = iter<MAX_ITER ? ((float)(iter)/(float)MAX_ITER) : 0.0;
	//tex.write(half4(color, 0.0, 0.0, 1.0), index, 0);	
}

