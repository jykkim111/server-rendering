#version 330 core
#define fDataRange  65536.0

uniform sampler3D volume_data;
uniform usampler3D mask_data;

uniform vec3 P_screen;
uniform vec3 w_dir;
uniform vec3 h_dir;
uniform vec3 n_dir;

uniform int N_x;
uniform int N_y;
uniform int N_z;

uniform float m_spacingX;
uniform float m_spacingY;
uniform float m_spacingZ;

uniform int w_cnt;
uniform int h_cnt;
uniform int n_cnt;

uniform float w_len;
uniform float h_len;
uniform float n_len;

uniform float window_level;
uniform float window_width;

uniform int thickness_mode;
uniform bool thickness;
uniform bool inverse;

uniform float mask_opacity;
uniform vec3 color_palette[9];
uniform int mask_visibility[9];

in vec3 vert_out;
out vec3 color;

float _applyWindowing(float v) {
	v -= window_level - window_width / 2;

	if (v < 0)
		v = 0;
	else if (v > window_width)
		v = window_width;

	if (!inverse) {
		v /= window_width;
	}
	else {
		v *= -1;
		v /= window_width;
		v += 1;
	}

	return v;
}

void main()
{
	uint cur_mask;

	if(!thickness){
		vec3 cur = P_screen +  w_len * w_dir * vert_out.x + h_len * h_dir * vert_out.y;
		float cur_intensity = fDataRange * texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;

		cur_mask = texture(mask_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;

		if(cur_mask == 0u || mask_visibility[cur_mask]==0){
			color = _applyWindowing(cur_intensity) * vec3(1.0f, 1.0f, 1.0f);
		}
		else{
			color = vec3(color_palette[cur_mask].g, color_palette[cur_mask].b, color_palette[cur_mask].r) * mask_opacity + _applyWindowing(cur_intensity) * vec3(1.0f, 1.0f, 1.0f) * (1-mask_opacity);
		}
	}
	else{
		int num = 1;
		vec3 total_intensity=vec3(0.0f, 0.0f, 0.0f);

		if(thickness_mode==1){
			total_intensity = vec3(1.0f, 1.0f, 1.0f);
		}

		for(int i=-n_cnt; i<n_cnt; i++){
			vec3 start = P_screen + i * n_len * n_dir;

			vec3 cur = start +  w_len * w_dir * vert_out.x + h_len * h_dir * vert_out.y;
			float cur_intensity = fDataRange * texture(volume_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;

			cur_mask = texture(mask_data, vec3(cur.x / (N_x * m_spacingX), cur.y / (N_y * m_spacingY), cur.z / (N_z * m_spacingZ))).x;

			if(cur_mask == 0u || mask_visibility[cur_mask]==0){
				if(thickness_mode==0){
					total_intensity += (_applyWindowing(cur_intensity) - total_intensity) / num * vec3(1.0f, 1.0f, 1.0f);
				}
				else if(thickness_mode==1){
					float temp_intensity = _applyWindowing(cur_intensity);
					if(temp_intensity < total_intensity.x){
						total_intensity = temp_intensity * vec3(1.0f, 1.0f, 1.0f);
					}
				}
				else if(thickness_mode==2){
					float temp_intensity = _applyWindowing(cur_intensity);
					if(temp_intensity > total_intensity.x){
						total_intensity = temp_intensity * vec3(1.0f, 1.0f, 1.0f);
					}
				}
			}
			else{
				total_intensity = color_palette[cur_mask];
				break;
			}
			num += 1;
		}

		color = vec3(total_intensity.g, total_intensity.b, total_intensity.r);
	}
}
