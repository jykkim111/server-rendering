#version 330 core
#define fDataRange  65536.0

uniform sampler3D volume_data;
uniform usampler3D mask_data;
uniform sampler1D tex_tf;
uniform usampler3D sculpt_data;

uniform int N_x;
uniform int N_y;
uniform int N_z;

uniform float fDeltaX;
uniform float fDeltaY;
uniform float fDeltaZ;

//uniform float slice_thickness;
uniform float m_spacingX;
uniform float m_spacingY;
uniform float m_spacingZ;
uniform vec3 P_screen;
uniform vec3 v_width;
uniform vec3 v_height;
uniform vec3 v_normal;
uniform float window_level;
uniform float window_width;
uniform bool skipping; // true = empty-space skipping, False = none
uniform float fOffset;
//uniform float max_intensity;

uniform bool shader_on;
uniform float m_specular;
uniform float m_brightness;

uniform int proj_type;
uniform float x1;
uniform float y1;
uniform float x2;
uniform float y2;
uniform float x3;
uniform float y3;
uniform float x4;
uniform float y4;

uniform vec4 color_palette[9];
uniform int mask_visibility[9];

uniform bool is_sculpt;

in vec3 vert_out;
out vec4 color;

float small_d, max_intensity, min_intensity, accumulated_opacity;
float fmax = -65535.0;
float fmin = 65535.0;
float fval;
float maxIP_max, minIP_max, raysum_max = -65535.0;
float maxIP_min,  minIP_min, raysum_min = 65535.0;
vec3 ray_start;
vec3 accumulated_intensity;
vec4 vFragColor;
float a, b, c;


bool is_penetrated(float x, float y, float z)
{
	int t = 4;
	if (t * m_spacingX < x && x < (N_x - t) * m_spacingX && 
	    t * m_spacingY < y && y < (N_y - t) * m_spacingY &&	
		t * m_spacingZ  < z && z < (N_z - t) * m_spacingZ)
		return true;

	return false;
}


void get_t_front_back(in float x_min, in float x_max, in float y_min, in float y_max, in float z_min, in float z_max, out float t0_x, out float t0_y, out float t0_z, out float t1_x, out float t1_y, out float t1_z)
{
	t0_x = (x_min - ray_start.x) / a;
	t1_x = (x_max - ray_start.x) / a;
	t0_y = (y_min - ray_start.y) / b;
	t1_y = (y_max - ray_start.y) / b;
	t0_z = (z_min - ray_start.z) / c;
	t1_z = (z_max - ray_start.z) / c;

	if (a == 0) {
		t0_x = -1.0 / 0.0;
		t1_x = 1.0 / 0.0;
		if (ray_start.x < x_min)
			t0_x = 1.0 / 0.0;
		if (x_max < ray_start.x)
			t0_x = -1.0 / 0.0;
	}
	if (b == 0) {
		t0_y = -1.0 / 0.0;
		t1_y = 1.0 / 0.0;
		if (ray_start.y < y_min)
			t0_y = 1.0 / 0.0;
		if (y_max < ray_start.x)
			t0_y = -1.0 / 0.0;
	}
	if (c == 0)	{
		t0_z = -1.0 / 0.0;
		t1_z = 1.0 / 0.0;
		if (ray_start.z < z_min)
			t0_z = 1.0 / 0.0;
		if (z_max < ray_start.x)
			t0_z = -1.0 / 0.0;
	}
}

vec4 cubic(float x){
	const float s = 0.5;
	float x2 = x * x;
	float x3 = x2 * x;
	vec4 w;
	w.x =    -s*x3 +     2*s*x2 - s*x + 0;
    w.y = (2-s)*x3 +   (s-3)*x2       + 1;
    w.z = (s-2)*x3 + (3-2*s)*x2 + s*x + 0;
    w.w =     s*x3 -       s*x2       + 0;
    return w;
}





float interpolate_tricubic(sampler3D tex, vec3 coord){
	
	vec3 nrOfVoxels = vec3(textureSize(tex, 0));
	vec3 coord_grid = coord * nrOfVoxels - 0.5;
	vec3 index = floor(coord_grid);
	vec3 fraction = coord_grid - index;
	vec3 one_frac = 1.0 - fraction;

	vec3 w0 = 1.0/6.0 * one_frac*one_frac*one_frac;
	vec3 w1 = 2.0/3.0 - 0.5 * fraction*fraction*(2.0-fraction);
	vec3 w2 = 2.0/3.0 - 0.5 * one_frac*one_frac*(2.0-one_frac);
	vec3 w3 = 1.0/6.0 * fraction*fraction*fraction;

	vec3 g0 = w0 + w1;
	vec3 g1 = w2 + w3;
	vec3 mult = 1.0 / nrOfVoxels;
	vec3 h0 = mult * ((w1 / g0) - 0.5 + index);  //h0 = w1/g0 - 1, move from [-0.5, nrOfVoxels-0.5] to [0,1]\n"
	vec3 h1 = mult * ((w3 / g1) + 1.5 + index);  //h1 = w3/g1 + 1, move from [-0.5, nrOfVoxels-0.5] to [0,1]\n"
	
	// fetch the eight linear interpolations
	// weighting and fetching is interleaved for performance and stability reasons
	float tex000 = texture(tex, h0).r;
	float tex100 = texture(tex, vec3(h1.x, h0.y, h0.z)).r;
	tex000 = mix(tex100, tex000, g0.x);  //weigh along the x-direction\n"
	float tex010 = texture(tex, vec3(h0.x, h1.y, h0.z)).r;
	float tex110 = texture(tex, vec3(h1.x, h1.y, h0.z)).r;
	tex010 = mix(tex110, tex010, g0.x);  //weigh along the x-direction\n"
	tex000 = mix(tex010, tex000, g0.y);  //weigh along the y-direction\n"
	float tex001 = texture(tex, vec3(h0.x, h0.y, h1.z)).r;
	float tex101 = texture(tex, vec3(h1.x, h0.y, h1.z)).r;
	tex001 = mix(tex101, tex001, g0.x);  //weigh along the x-direction\n"
	float tex011 = texture(tex, vec3(h0.x, h1.y, h1.z)).r;
	float tex111 = texture(tex, h1).r;
	tex011 = mix(tex111, tex011, g0.x);  //weigh along the x-direction\n"
	tex001 = mix(tex011, tex001, g0.y);  //weigh along the y-direction\n"

	return mix(tex001, tex000, g0.z);  //weigh along the z-direction\n"

}


void sample_through(in float t0, in float t1)
{
	vec3 cur = P_screen + v_width * vert_out.x + v_height * vert_out.y + t0 * v_normal;
	float cur_t = t0;
	float fValPrev, fValCurr, cur_intensity, cur_opacity, dx, dy, dz, cur_shading;
	vec4 vOTF, vOTFCurr, vOTFPrev;
	uint cur_mask;
	
	if(proj_type == 1){
		while (cur_t < t1) {
			//fval = textureBicubic(volume_data, )
			fval = texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).r;
			if (fval >= fmax) {
				fmax = fval;
			}

			cur = cur + v_normal;
			cur_t += length(v_normal);
		}
		
		float hu = fDataRange * fmax + fOffset;
		if (maxIP_max <= hu){
			maxIP_max = hu;
		}
		
		if(maxIP_min >= hu){
			maxIP_min = hu;
		}
		//if(hu <= window_level + fOffset -0.5 - (window_width-1.0f)*0.5) {
		//	hu = 0.0;
		//}
		//else if(hu >window_level + fOffset -0.5 + (window_width-1.0f)*0.5) {
		//	hu = 255.0;
		//}
		//else {
		//hu =  (hu - maxIP_min) * 255.0 / (maxIP_max - maxIP_min) ;
		//}
		hu =  (hu + 991) * 255.0 / (1596 + 991) ;
		//hu = (hu + 991) * 255.0 / (max_intensity + 991);
		float retVal = hu/255.0;
		//float retVal = hu/255.0;
		//float retVal = clamp((hu - window_level + window_width / 2) / (window_width + 500), 0.0f, 1.0f);
		vFragColor = vec4(retVal, retVal, retVal, 1.0);
	
	}
	else if (proj_type == 2){
		
		while (cur_t < t1) {
			fval = texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).r;
			
			
			if(fmin >= fval){
				//float hu = fDataRange * fval + fOffset;
				fmin = fval;
				
			}
			
			cur = cur + v_normal;
			cur_t += length(v_normal);
		}

		float min_hu = fDataRange * fmin + fOffset;

		//if(min_hu < -700) {
		//	min_hu = 0.0;
		//}
		//else if(min_hu > 300) {
		//	min_hu = 0.0;
		//}
		//else if(min_hu > 100){
		//	min_hu = 255;
		//}
		//else if(min_hu >= -100 && min_hu < 300){
		//	//min_hu = (min_hu + 700)* 255 /(100);
		//	min_hu = 255.0;
		//}
		//else{
		//	min_hu =  (min_hu + 700)* 255  /(1000);
		//}
		
		min_hu =  (min_hu + 1024)* 255  /(1142);
		
		float retVal = min_hu/255.0;
		//float retVal = hu/255.0;
		//float retVal = clamp((hu - window_level + window_width / 2) / (window_width + 500), 0.0f, 1.0f);
		vFragColor = vec4(retVal, retVal, retVal, 1.0);



	}
	else if(proj_type == 3){
		float total_intensity = 0.0;
		int count = 0;
		while (cur_t < t1) {
			fval = texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).r;
			
					
			cur_intensity = fDataRange * fval + fOffset;
			total_intensity += cur_intensity;
			count += 1;
			//if(cur_intensity > -1000){
			//	
			//}
			
			
			cur = cur + v_normal;
			cur_t += length(v_normal);
			
		}
		
		float avg_intensity = total_intensity / count;
		float retVal =  (avg_intensity +1005)* 255.0 / (1334);
		
		retVal = retVal/255.0;
		vFragColor = vec4(retVal, retVal, retVal, 1.0);
	}
	else{

		//fValPrev = fDataRange * texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;
		fValPrev = fDataRange * interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ)));
		
		vOTFPrev = texelFetch(tex_tf, int(fValPrev), 0);
		
		while (cur_t < t1) { // stop when reached end
		// sample from data & calculate shadow
			//fValCurr = fDataRange * texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;
			fValCurr = fDataRange * interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ)));
			
			vOTFCurr = texelFetch(tex_tf, int(fValCurr), 0);
			cur_mask = texture(mask_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;

			if(is_sculpt){
				uint cur_sculpt = texture(sculpt_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;

				if(cur_sculpt == 1u){
					fValCurr = 0f;
					cur_mask = 0u;
				}
			}


			if(cur_mask == 0u || mask_visibility[cur_mask]==0){
				
				if(int(fValCurr) - int(fValPrev) == 0){
					vec4 pOTF = texelFetch(tex_tf, int(fValCurr), 0);
                    vec4 cOTF = texelFetch(tex_tf, int(fValCurr + 1.0), 0);
					vOTF = cOTF-pOTF;
				}
				else if( fValCurr > fValPrev){
					int d= int(fValCurr) - int(fValPrev);
					vOTF = (vOTFCurr - vOTFPrev)/d;
				}
				else{
                    int d = int(fValPrev) - int(fValCurr);
                    vOTF = (vOTFPrev - vOTFCurr)/d;
				}
				
				fValPrev = fValCurr;				
				vOTFPrev = vOTFCurr;
				
				if(vOTF.a > 0){
					if(shader_on){
						dx = interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX) + small_d, cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ)))
							 - interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX) - small_d, cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ)));
						dy = interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY) + small_d, cur.z / (N_z * m_spacingZ))) 
							 - interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY) - small_d, cur.z / (N_z * m_spacingZ)));
						dz = interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ) + small_d)) 
							 - interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ) - small_d));
						cur_shading = clamp(m_brightness + m_specular *abs(dot(normalize(vec3(dx, dy, dz) / (2 * small_d)), normalize(v_normal))), 0.0f, 1.0f);
					
						vOTF.rgb *= cur_shading * cur_shading;
					}
					vOTF.a *= exp(-3.0 * (1.0 - vOTF.a));
					vOTF.rgb *= vOTF.a;
					vFragColor = vFragColor + (1.0-vFragColor.a)*vOTF;
					if(vFragColor.a > 0.99) break; 
				}
			}	

			else{
				vec4 mask_color;

				if(int(fValCurr) - int(fValPrev) == 0){
					vec4 pOTF = texelFetch(tex_tf, int(fValCurr), 0);
                    vec4 cOTF = texelFetch(tex_tf, int(fValCurr + 1.0), 0);
					vOTF = cOTF-pOTF;
				}
				else if( fValCurr > fValPrev){
					int d= int(fValCurr) - int(fValPrev);
					vOTF = (vOTFCurr - vOTFPrev)/d;
				}
				else{
                    int d = int(fValPrev) - int(fValCurr);
                    vOTF = (vOTFPrev - vOTFCurr)/d;
				}
				
				fValPrev = fValCurr;				
				vOTFPrev = vOTFCurr;
				
				
				
				mask_color = vec4(color_palette[cur_mask].g, color_palette[cur_mask].r, color_palette[cur_mask].b, 0.0);
				mask_color.a = color_palette[cur_mask].a;
				//mask_color.a = vOTF.a;

				if(shader_on){
					dx = interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX) + small_d, cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ)))
						 - interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX) - small_d, cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ)));
					dy = interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY) + small_d, cur.z / (N_z * m_spacingZ))) 
						 - interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY) - small_d, cur.z / (N_z * m_spacingZ)));
					dz = interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ) + small_d)) 
						 - interpolate_tricubic(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ) - small_d));
					cur_shading = clamp(m_specular + m_brightness *abs(dot(normalize(vec3(dx, dy, dz) / (2 * small_d)), normalize(v_normal))), 0.0f, 1.0f);
				
					mask_color.rgb *= cur_shading * cur_shading;
				}
				mask_color.a *= exp(-3.0 * (1.0 - mask_color.a));
				mask_color.rgb *= mask_color.a;
				vFragColor = vFragColor + (1.0-vFragColor.a)*mask_color;
				if(vFragColor.a > 0.99) break; 

			}
				
			
			 	
			
			
			// go to next sampling point
			cur = cur + v_normal * 0.1;
			cur_t += length(v_normal * 0.1);
		}
	}
}


void main()
{
	// set initial values
	ray_start = P_screen + v_width * vert_out.x + v_height * vert_out.y;
	small_d = 0.5f / max(max(N_x * m_spacingX, N_y * m_spacingY), N_z * m_spacingZ);
	max_intensity = 0.0f;
	accumulated_opacity = 1.0f;
	//accumulated_intensity = vec3(0.0, 0.0, 1.0);
	vFragColor = vec4(0.0, 0.0, 0.0, 0.0);
	a = v_normal.x;
	b = v_normal.y;
	c = v_normal.z;

	int child_num_shifter = 0;
	float t_front, t_back, t0_x, t0_y, t0_z, t1_x, t1_y, t1_z;
	float x_min, x_max, y_min, y_max, z_min, z_max;
	x_min = 0;
	y_min = 0;
	z_min = 0;
	x_max = N_x * m_spacingX - 1;
	y_max = N_y * m_spacingY - 1;
	z_max = N_z * m_spacingZ - 1;
	if (a < 0) {
		x_min = N_x * m_spacingX - 1;
		x_max = 0;
		child_num_shifter += 1;
	}
	if (b < 0) {
		y_min = N_y * m_spacingY - 1;
		y_max = 0;
		child_num_shifter += 2;
	}
	if (c < 0) {
		z_min = N_z * m_spacingZ - 1;
		z_max = 0;
		child_num_shifter += 4;
	}

	// examine first node
	get_t_front_back(x_min, x_max, y_min, y_max, z_min, z_max, t0_x, t0_y, t0_z, t1_x, t1_y, t1_z);
	t_front = max(t0_x, max(t0_y, t0_z));
	t_back = min(t1_x, min(t1_y, t1_z));
	if (t_front <= t_back) { // ray intersects with data
		sample_through(t_front, t_back);
	}

	// color value
	color = vec4(vFragColor.r, vFragColor.b, vFragColor.g, vFragColor.a);
}
