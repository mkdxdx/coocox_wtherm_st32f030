#include "jsontrie.h"

// struct tails
const JSON_STRUCT node_3h = {'\0',JB_3H,0,0};
const JSON_STRUCT node_all = {'\0',JB_ALL,0,0};
const JSON_STRUCT node_city = {'\0',JB_CITY,0,0};
const JSON_STRUCT node_clouds = {'\0',JB_CLOUDS,0,0};
const JSON_STRUCT node_cnt = {'\0',JB_CNT,0,0};
const JSON_STRUCT node_cod = {'\0',JB_COD,0,0};
const JSON_STRUCT node_coord = {'\0',JB_COORD,0,0};
const JSON_STRUCT node_country = {'\0',JB_COUNTRY,0,0};
const JSON_STRUCT node_deg = {'\0',JB_DEG,0,0};
const JSON_STRUCT node_desc = {'\0',JB_DESC,0,0};
const JSON_STRUCT node_dt = {'\0',JB_DT,0,0};
const JSON_STRUCT node_dt_txt = {'\0',JB_DT_TXT,0,0};
const JSON_STRUCT node_grnd_level = {'\0',JB_GRND_LEVEL,0,0};
const JSON_STRUCT node_humidity = {'\0',JB_HUMIDITY,0,0};
const JSON_STRUCT node_icon = {'\0',JB_ICON,0,0};
const JSON_STRUCT node_id = {'\0',JB_ID,0,0};
const JSON_STRUCT node_lat = {'\0',JB_LAT,0,0};
const JSON_STRUCT node_list = {'\0',JB_LIST,0,0};
const JSON_STRUCT node_lon = {'\0',JB_LON,0,0};
const JSON_STRUCT node_main = {'\0',JB_MAIN,0,0};
const JSON_STRUCT node_message = {'\0',JB_MESSAGE,0,0};
const JSON_STRUCT node_name = {'\0',JB_NAME,0,0};
const JSON_STRUCT node_pod = {'\0',JB_POD,0,0};
const JSON_STRUCT node_pressure = {'\0',JB_PRESSURE,0,0};
const JSON_STRUCT node_rain = {'\0',JB_RAIN,0,0};
const JSON_STRUCT node_sea_level = {'\0',JB_SEA_LEVEL,0,0};
const JSON_STRUCT node_speed = {'\0',JB_SPEED,0,0};
const JSON_STRUCT node_sys = {'\0',JB_SYS,0,0};
const JSON_STRUCT node_temp = {'\0',JB_TEMP,0,0};
const JSON_STRUCT node_temp_kf = {'\0',JB_TEMP_KF,0,0};
const JSON_STRUCT node_temp_max = {'\0',JB_TEMP_MAX,0,0};
const JSON_STRUCT node_temp_min = {'\0',JB_TEMP_MIN,0,0};
const JSON_STRUCT node_weather = {'\0',JB_WEATHER,0,0};
const JSON_STRUCT node_wind = {'\0',JB_WIND,0,0};

// wind
NODEN(node_wind_d,'d',&node_wind,0);
NODEN(node_wind_n,'n',&node_wind_d,0);
NODEN(node_wind_i,'i',&node_wind_n,0);
// weather
NODEN(node_weather_r,'r',&node_weather,0);
NODEN(node_weather_e2,'e',&node_weather_r,0);
NODEN(node_weather_h,'h',&node_weather_e2,0);
NODEN(node_weather_t,'t',&node_weather_h,0);
NODEN(node_weather_a,'a',&node_weather_t,0);
NODEN(node_weather_e,'e',&node_weather_a,&node_wind_i);
NODEN(node_weather_w,'w',&node_weather_e,0);
// temp_kf
NODEN(node_temp_kf_f,'f',&node_temp_kf,0);
NODEN(node_temp_kf_k,'k',&node_temp_kf_f,0);
// temp_max
NODEN(node_temp_max_x,'x',&node_temp_max,0);
NODEN(node_temp_max_a,'a',&node_temp_max_x,0);
// temp_min
NODEN(node_temp_min_n,'n',&node_temp_min,0);
NODEN(node_temp_min_i,'i',&node_temp_min_n,&node_temp_max_a);
NODEN(node_temp_minmax_m,'m',&node_temp_min_i,&node_temp_kf_k);
NODEN(node_temp_minmax_us,'_',&node_temp_minmax_m,&node_temp);
// temp
NODEN(node_temp_p,'p',&node_temp_minmax_us,0);
NODEN(node_temp_m,'m',&node_temp_p,0);
NODEN(node_temp_e,'e',&node_temp_m,0);
NODEN(node_temp_t,'t',&node_temp_e,&node_weather_w);
// sys
NODEN(node_sys_s,'s',&node_sys,0);
NODEN(node_sys_y,'y',&node_sys_s,0);
// speed
NODEN(node_speed_d,'d',&node_speed,0);
NODEN(node_speed_e2,'e',&node_speed_d,0);
NODEN(node_speed_e,'e',&node_speed_e2,0);
NODEN(node_speed_p,'p',&node_speed_e,&node_sys_y);
// sea_level
NODEN(node_sea_level_l2,'l',&node_sea_level,0);
NODEN(node_sea_level_e3,'e',&node_sea_level_l2,0);
NODEN(node_sea_level_v,'v',&node_sea_level_e3,0);
NODEN(node_sea_level_e2,'e',&node_sea_level_v,0);
NODEN(node_sea_level_l,'l',&node_sea_level_e2,0);
NODEN(node_sea_level_us,'_',&node_sea_level_l,0);
NODEN(node_sea_level_a,'a',&node_sea_level_us,0);
NODEN(node_sea_level_e,'e',&node_sea_level_a,&node_speed_p);
NODEN(node_sea_level_s,'s',&node_sea_level_e,&node_temp_t);
// rain
NODEN(node_rain_n,'n',&node_rain,0);
NODEN(node_rain_i,'i',&node_rain_n,0);
NODEN(node_rain_a,'a',&node_rain_i,0);
NODEN(node_rain_r,'r',&node_rain_a,&node_sea_level_s);
// pressure
NODEN(node_pressure_e2,'e',&node_pressure,0);
NODEN(node_pressure_r2,'r',&node_pressure_e2,0);
NODEN(node_pressure_u,'u',&node_pressure_r2,0);
NODEN(node_pressure_s2,'s',&node_pressure_u,0);
NODEN(node_pressure_s,'s',&node_pressure_s2,0);
NODEN(node_pressure_e,'e',&node_pressure_s,0);
NODEN(node_pressure_r,'r',&node_pressure_e,0);
// pod
NODEN(node_pod_d,'d',&node_pod,0);
NODEN(node_pod_o,'o',&node_pod_d,&node_pressure_r);
NODEN(node_pod_p,'p',&node_pod_o,&node_rain_r);
// name
NODEN(node_name_e,'e',&node_name,0);
NODEN(node_name_m,'m',&node_name_e,0);
NODEN(node_name_a,'a',&node_name_m,0);
NODEN(node_name_n,'n',&node_name_a,&node_pod_p);
// message
NODEN(node_message_e2,'e',&node_message,0);
NODEN(node_message_g,'g',&node_message_e2,0);
NODEN(node_message_a,'a',&node_message_g,0);
NODEN(node_message_s2,'s',&node_message_a,0);
NODEN(node_message_s,'s',&node_message_s2,0);
NODEN(node_message_e,'e',&node_message_s,0);
// main
NODEN(node_main_n,'n',&node_main,0);
NODEN(node_main_i,'i',&node_main_n,0);
NODEN(node_main_a,'a',&node_main_i,&node_message_e);
NODEN(node_main_m,'m',&node_main_a,&node_name_n);
// list
NODEN(node_list_t,'t',&node_list,0);
NODEN(node_list_s,'s',&node_list_t,0);
NODEN(node_list_i,'i',&node_list_s,0);
// lon
NODEN(node_lon_n,'n',&node_lon,0);
NODEN(node_lon_o,'o',&node_lon_n,&node_list_i);
// lat
NODEN(node_lat_t,'t',&node_lat,0);
NODEN(node_lat_a,'a',&node_lat_t,&node_lon_o);
NODEN(node_lat_l,'l',&node_lat_a,&node_main_m);
// id
NODEN(node_id_d,'d',&node_id,0);
// icon
NODEN(node_icon_n,'n',&node_icon,0);
NODEN(node_icon_o,'o',&node_icon_n,0);
NODEN(node_icon_c,'c',&node_icon_o,&node_id_d);
NODEN(node_icon_i,'i',&node_icon_c,&node_lat_l);
// humidity
NODEN(node_humidity_y,'y',&node_humidity,0);
NODEN(node_humidity_t,'t',&node_humidity_y,0);
NODEN(node_humidity_i2,'i',&node_humidity_t,0);
NODEN(node_humidity_d,'d',&node_humidity_i2,0);
NODEN(node_humidity_i,'i',&node_humidity_d,0);
NODEN(node_humidity_m,'m',&node_humidity_i,0);
NODEN(node_humidity_u,'u',&node_humidity_m,0);
NODEN(node_humidity_h,'h',&node_humidity_u,&node_icon_i);
// grnd_level
NODEN(node_grnd_level_l2,'l',&node_grnd_level,0);
NODEN(node_grnd_level_e2,'e',&node_grnd_level_l2,0);
NODEN(node_grnd_level_v,'v',&node_grnd_level_e2,0);
NODEN(node_grnd_level_e,'e',&node_grnd_level_v,0);
NODEN(node_grnd_level_l,'l',&node_grnd_level_e,0);
NODEN(node_grnd_level_us,'_',&node_grnd_level_l,0);
NODEN(node_grnd_level_d,'d',&node_grnd_level_us,0);
NODEN(node_grnd_level_n,'n',&node_grnd_level_d,0);
NODEN(node_grnd_level_r,'r',&node_grnd_level_n,0);
NODEN(node_grnd_level_g,'g',&node_grnd_level_r,&node_humidity_h);
// d_t_txt
NODEN(node_dt_txt_t3,'t',&node_dt_txt,0);
NODEN(node_dt_txt_x,'x',&node_dt_txt_t3,0);
NODEN(node_dt_txt_t2,'t',&node_dt_txt_x,0);
NODEN(node_dt_txt_us,'_',&node_dt_txt_t2,&node_dt);
NODEN(node_dt_txt_t,'t',&node_dt_txt_us,0);
// d_esc
NODEN(node_desc_c,'c',&node_desc,0);
NODEN(node_desc_s,'s',&node_desc_c,0);
// d_eg
NODEN(node_deg_g,'g',&node_deg,&node_desc_s);
NODEN(node_deg_e,'e',&node_deg_g,&node_dt_txt_t);
NODEN(node_deg_d,'d',&node_deg_e,&node_grnd_level_g);
// c_ountry
NODEN(node_country_y,'y',&node_country,0);
NODEN(node_country_r,'r',&node_country_y,0);
NODEN(node_country_t,'t',&node_country_r,0);
NODEN(node_country_n,'n',&node_country_t,0);
NODEN(node_country_u,'u',&node_country_n,0);
// c_oord
NODEN(node_coord_d,'d',&node_coord,0);
NODEN(node_coord_r,'r',&node_coord_d,0);
NODEN(node_coord_o,'o',&node_coord_r,&node_country_u);
// c_od
NODEN(node_cod_d,'d',&node_cod,&node_coord_o);
NODEN(node_cod_o,'o',&node_cod_d,0);
// c_nt
NODEN(node_cnt_t,'t',&node_cnt,0);
NODEN(node_cnt_n,'n',&node_cnt_t,&node_cod_o);
// c_louds
NODEN(node_clouds_s,'s',&node_clouds,0);
NODEN(node_clouds_d,'d',&node_clouds_s,0);
NODEN(node_clouds_u,'u',&node_clouds_d,0);
NODEN(node_clouds_o,'o',&node_clouds_u,0);
NODEN(node_clouds_l,'l',&node_clouds_o,&node_cnt_n);
// c_ity
NODEN(node_city_y,'y',&node_city,0);
NODEN(node_city_t,'t',&node_city_y,0);
NODEN(node_city_i,'i',&node_city_t,&node_clouds_l);
NODEN(node_city_c,'c',&node_city_i,&node_deg_d);
// all
NODEN(node_all_l2,'l',&node_all,0);
NODEN(node_all_l,'l',&node_all_l2,0);
NODEN(node_all_a,'a',&node_all_l,&node_city_c);
// 3h
NODEN(node_3h_h,'h',&node_3h,0);
NODEN(node_3h_3,'3',&node_3h_h,&node_all_a);	// our root node




enum E_JSONBLOCK_ID parseJSONString(char * str) {
	struct JSON_STRUCT * root = &node_3h_3;
	return getid(root, str);
}

enum E_JSONBLOCK_ID getid(JSON_STRUCT * node, char * str) {
	if (*str == node->key) {
		if (node->key == '\0') {
			return node->bnum;
		} else {
			if (node->child != 0) {
				return getid(node->child,++str);
			} else {
				return JB_NONE;
			}
		}
	} else {
		if (node->next) {
			return getid(node->next,str);
		} else {
			return JB_NONE;
		}
	}
	// fallback
	return JB_NONE;
}


JSON_STRUCT * getnext(JSON_STRUCT * node, char c) {
	if (node->key == '\0') {
		return node;
	}
	if (node->key == c) {
		if (node->child != 0) {
			return node->child;
		}
	} else {
		if (node->next != 0) {
			return getnext(node->next,c);
		}
	}
	return 0;
}
