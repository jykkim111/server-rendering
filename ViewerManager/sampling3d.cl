#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__kernel void upsample3d(__global short *data, __global short *dest,
	int width, int height, int depth, float down_ratio) {

	int tid = get_global_id(0);
	int z = tid / (height * width);
	int y = (tid % (height * width)) / width;
	int x = (tid % (height * width)) % width;

	int down_width = width / down_ratio;
	int down_height = height / down_ratio;
	int down_depth = depth / down_ratio;

	int down_z = min((int)round(z / down_ratio), down_depth - 1);
	int down_y = min((int)round(y / down_ratio), down_height - 1);
	int down_x = min((int)round(x / down_ratio), down_width - 1);

	short val = data[down_z * down_width * down_height + down_y * down_width + down_x];
	dest[tid] = val;
}

float lerp(float v1, float v2, float x) {
	return v1 * (1 - x) + v2 * x;
}

//__kernel void downsample3d(__global short *vol, __global short *mask,
//	__global short *vol_down, __global short *mask_down,
//	int width, int height, int depth, float down_ratio) {

__kernel void downsample3d(__global short *vol, __global short *vol_down,
	__global short *mask, __global short *mask_down,
	int width, int height, int depth, float down_ratio) {

	int tid = get_global_id(0);
	int z = tid / (height * width);
	int y = (tid % (height * width)) / width;
	int x = (tid % (height * width)) % width;

	int down_width = width / down_ratio;
	int down_height = height / down_ratio;
	int down_depth = depth / down_ratio;

	int down_z = min((int)round(z / down_ratio), down_depth - 1);
	int down_y = min((int)round(y / down_ratio), down_height - 1);
	int down_x = min((int)round(x / down_ratio), down_width - 1);

	short mask_val = mask[tid];
	short mask_down_val = mask_down[down_z * down_width * down_height + down_y * down_width + down_x];
	if (mask_down_val == 0) {
		mask_down[down_z * down_width * down_height + down_y * down_width + down_x] = mask_val;
		//printf("%d\n", mask_down[down_z * down_width * down_height + down_y * down_width + down_x]);
	}

	short vol_val = vol[tid];
	short vol_down_val = vol_down[down_z * down_width * down_height + down_y * down_width + down_x];
	//if (vol_down_val == 0) {
	vol_down[down_z * down_width * down_height + down_y * down_width + down_x] = vol_val;
	//}
}