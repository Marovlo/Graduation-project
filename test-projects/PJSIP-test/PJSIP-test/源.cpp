#include <pjlib.h>
#include <pjlib-util.h>
#include <pjnath.h>
#include <pjsip.h>
#include <pjsip_ua.h>
#include <pjsip_simple.h>
#include <pjsua-lib/pjsua.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include<iostream>
using namespace std;
#define PJ_WIN32 1

pj_caching_pool cache_pool;//内存池工厂
pj_pool_t* pool = nullptr;//内存池
pjmedia_endpt* media_endpt = nullptr;//媒体端点
pjmedia_stream* g_sound_stream = nullptr;
pjmedia_snd_port* g_sound_port = nullptr;
pj_status_t init();//程序开始之前的初始化

pj_status_t create_sound_port(pjmedia_stream* media_stream, 
	pjmedia_snd_port** sound_port);//构建音频流端口

pj_status_t create_audio_stream(const pjmedia_codec_info* codec_info, 
	pjmedia_dir media_dir,pj_uint16_t local_port, 
	pj_sockaddr_in* remote_addr, pjmedia_stream** media_stream);//初始化音频流

pj_status_t sound_stream_create(unsigned short local_port, char* remote_ip,
	unsigned short remote_port);//连接音频流




pj_status_t sound_stream_create(unsigned short local_port, char* remote_ip,
		unsigned short remote_port)//连接音频流
{
	if (!g_sound_stream)
	{
		pj_status_t status;

		pj_sockaddr_in remote_addr;//设置远端地址
		pj_bzero(&remote_addr, sizeof(remote_addr));
		pj_str_t ip = pj_str(remote_ip);
		pj_uint16_t port = (pj_uint16_t)remote_port;
		status = pj_sockaddr_in_init(&remote_addr, &ip, port);
		if (status != PJ_SUCCESS);//判断失败

		const pjmedia_codec_info* codec_info = nullptr;// 获取音频格式PCMU的编解码信息
		//目前只开启了"PCMU/8000/1" "PCMA/8000/1" "G722/16000/1"
		unsigned count = 1;
		char codecid[] = "PCMU";
		pj_str_t codec_id = pj_str(codecid);
		pjmedia_codec_mgr* codec_manager = pjmedia_endpt_get_codec_mgr(media_endpt);
		//这个函数是给codec_info根据我们选的编码方式（在这里是PCMU）进行初始化****
		status = pjmedia_codec_mgr_find_codecs_by_id(codec_manager, &codec_id, &count, &codec_info, nullptr);
		if (status != PJ_SUCCESS);//判断失败

		pjmedia_dir media_dir = PJMEDIA_DIR_ENCODING_DECODING;
		pjmedia_stream* media_stream = nullptr;//创建音频流
		status = create_audio_stream(codec_info, media_dir, local_port, &remote_addr, &media_stream);

		//创建声音设备
		pjmedia_snd_port* sound_port = nullptr;
		status = create_sound_port(media_stream, &sound_port);
		if (status != PJ_SUCCESS);

		//启动音频流
		status = pjmedia_stream_start(media_stream);
		if (status != PJ_SUCCESS);
		g_sound_stream = media_stream;
		g_sound_port = sound_port;
		return status;
	}
	return -1;
}

pj_status_t create_audio_stream(const pjmedia_codec_info* codec_info, pjmedia_dir media_dir,
		pj_uint16_t local_port, pj_sockaddr_in* remote_addr, pjmedia_stream** media_stream)//初始化音频流
{
	pjmedia_stream_info stream_info;//音频流信息，用于创建音频流
	pjmedia_transport* media_transport = nullptr;// 媒体传输，用于传输音频流
	pj_status_t status;

	//初始化音频流信息
	pj_bzero(&stream_info, sizeof(stream_info));
	//对这句话有疑问，为啥非要bzero，这是一个栈上的结构体，又不是堆上的
	//后期验证，必须要bzero，在pjmedia_stream_create中有先判断stream_info.prama是否为null的操作
	//如果不为null，就会对其进行一个memcpy，但是这个prama本身是没有初始化的，是0xccccccc，memcpy会报错
	stream_info.type = PJMEDIA_TYPE_AUDIO;
	stream_info.dir = media_dir;
	//将传入的codec_info赋值给stream_info中的fmt（fmt就是一个codec_info）
	pj_memcpy(&stream_info.fmt, codec_info, sizeof(pjmedia_codec_info));
	stream_info.tx_pt = codec_info->pt;
	stream_info.rx_pt = codec_info->pt;
	stream_info.ssrc = pj_rand();
	//远程地址相关的信息保存在音频流信息中
	pj_memcpy(&stream_info.rem_addr, remote_addr, sizeof(pj_sockaddr_in));
	status = pjmedia_transport_udp_create(media_endpt, nullptr, local_port, 0, &media_transport);
	if (status != PJ_SUCCESS);
	status = pjmedia_stream_create(media_endpt, pool, &stream_info, media_transport, nullptr, media_stream);
	if (status != PJ_SUCCESS);
	//开始传输
	pjmedia_transport_media_start(media_transport, nullptr, nullptr, nullptr, 0);
	return PJ_SUCCESS;
}
pj_status_t create_sound_port(pjmedia_stream* media_stream, pjmedia_snd_port** sound_port)//构建音频流端口
{
	if (!media_stream || !sound_port)return -1;
	pj_status_t status;
	pjmedia_port* media_port = nullptr;//音频流信息
	pjmedia_stream_info stream_info;//音频流信息
	pjmedia_snd_port* temp_sound_port = nullptr;//临时声音设备端口
	status = pjmedia_stream_get_port(media_stream, &media_port);
	if (status != PJ_SUCCESS)cout << "pjmedia_stream_get_port failed\n";;
	status = pjmedia_stream_get_info(media_stream, &stream_info);
	if (status != PJ_SUCCESS)cout << "pjmedia_stream_get_info failed\n";;
	// 根据音频流方向创建声音设备
	if (stream_info.dir = PJMEDIA_DIR_ENCODING_DECODING)
	{
		status = pjmedia_snd_port_create(pool, -1, -1,
			PJMEDIA_PIA_SRATE(&media_port->info),
			PJMEDIA_PIA_CCNT(&media_port->info),
			PJMEDIA_PIA_SPF(&media_port->info),
			PJMEDIA_PIA_BITS(&media_port->info),
			0, &temp_sound_port);
	}
	else if (stream_info.dir == PJMEDIA_DIR_ENCODING)
	{
		status = pjmedia_snd_port_create_rec(pool, -1,
			PJMEDIA_PIA_SRATE(&media_port->info),
			PJMEDIA_PIA_CCNT(&media_port->info),
			PJMEDIA_PIA_SPF(&media_port->info),
			PJMEDIA_PIA_BITS(&media_port->info),
			0, &temp_sound_port);
	}
	else
	{
		status = pjmedia_snd_port_create_player(pool, -1,
			PJMEDIA_PIA_SRATE(&media_port->info),
			PJMEDIA_PIA_CCNT(&media_port->info),
			PJMEDIA_PIA_SPF(&media_port->info),
			PJMEDIA_PIA_BITS(&media_port->info),
			0, &temp_sound_port);
	}

	//连接声音设备和音频流
	status = pjmedia_snd_port_connect(temp_sound_port, media_port);
	if (status != PJ_SUCCESS);
	*sound_port = temp_sound_port;
	return PJ_SUCCESS;

}
pj_status_t init()//程序开始之前的初始化
{
	pj_status_t status;
	status = pj_init();//初始化pjlib,必须写
	pj_caching_pool_init(&cache_pool, &pj_pool_factory_default_policy, 0);//创建内存池工厂(不是创建内存池,是创建创建内存池的工厂)
	//todo 这个0的max capacity有疑问.
	if (status != PJ_SUCCESS)cout << "pj_init failed\n";
	pool = pj_pool_create(&cache_pool.factory, "PJSIP TEST", 4000, 4000, nullptr);
	status = pjmedia_event_mgr_create(pool, 0, nullptr);//创建event manager，第三个参数为管理该manager的指针，没有用指针去接受
	if (status != PJ_SUCCESS)cout << "pjmedia_event_mgr_create failed\n";
	status = pjmedia_endpt_create(&cache_pool.factory, nullptr, 1, &media_endpt);// 创建媒体端点 开启1个工作线程用于poll io
	if (status != PJ_SUCCESS)cout << "pjmedia_endpt_create failed\n";
	status = pjmedia_codec_register_audio_codecs(media_endpt, nullptr);// 注册所有支持的音频编解码器                                                                            c]
	if (status != PJ_SUCCESS)cout << "pjmedia_codec_register_audio_codecs failed\n";
	//pj_shutdown();
	return status;
}//




int main(void)
{
	init();
	char ip[] = "127.0.0.1";
	sound_stream_create(4000, ip, 4000);

	return 0;
}
