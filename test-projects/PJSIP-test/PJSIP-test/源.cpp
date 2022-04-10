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

pj_caching_pool cache_pool;//�ڴ�ع���
pj_pool_t* pool = nullptr;//�ڴ��
pjmedia_endpt* media_endpt = nullptr;//ý��˵�
pjmedia_stream* g_sound_stream = nullptr;
pjmedia_snd_port* g_sound_port = nullptr;
pj_status_t init();//����ʼ֮ǰ�ĳ�ʼ��

pj_status_t create_sound_port(pjmedia_stream* media_stream, 
	pjmedia_snd_port** sound_port);//������Ƶ���˿�

pj_status_t create_audio_stream(const pjmedia_codec_info* codec_info, 
	pjmedia_dir media_dir,pj_uint16_t local_port, 
	pj_sockaddr_in* remote_addr, pjmedia_stream** media_stream);//��ʼ����Ƶ��

pj_status_t sound_stream_create(unsigned short local_port, char* remote_ip,
	unsigned short remote_port);//������Ƶ��




pj_status_t sound_stream_create(unsigned short local_port, char* remote_ip,
		unsigned short remote_port)//������Ƶ��
{
	if (!g_sound_stream)
	{
		pj_status_t status;

		pj_sockaddr_in remote_addr;//����Զ�˵�ַ
		pj_bzero(&remote_addr, sizeof(remote_addr));
		pj_str_t ip = pj_str(remote_ip);
		pj_uint16_t port = (pj_uint16_t)remote_port;
		status = pj_sockaddr_in_init(&remote_addr, &ip, port);
		if (status != PJ_SUCCESS);//�ж�ʧ��

		const pjmedia_codec_info* codec_info = nullptr;// ��ȡ��Ƶ��ʽPCMU�ı������Ϣ
		//Ŀǰֻ������"PCMU/8000/1" "PCMA/8000/1" "G722/16000/1"
		unsigned count = 1;
		char codecid[] = "PCMU";
		pj_str_t codec_id = pj_str(codecid);
		pjmedia_codec_mgr* codec_manager = pjmedia_endpt_get_codec_mgr(media_endpt);
		//��������Ǹ�codec_info��������ѡ�ı��뷽ʽ����������PCMU�����г�ʼ��****
		status = pjmedia_codec_mgr_find_codecs_by_id(codec_manager, &codec_id, &count, &codec_info, nullptr);
		if (status != PJ_SUCCESS);//�ж�ʧ��

		pjmedia_dir media_dir = PJMEDIA_DIR_ENCODING_DECODING;
		pjmedia_stream* media_stream = nullptr;//������Ƶ��
		status = create_audio_stream(codec_info, media_dir, local_port, &remote_addr, &media_stream);

		//���������豸
		pjmedia_snd_port* sound_port = nullptr;
		status = create_sound_port(media_stream, &sound_port);
		if (status != PJ_SUCCESS);

		//������Ƶ��
		status = pjmedia_stream_start(media_stream);
		if (status != PJ_SUCCESS);
		g_sound_stream = media_stream;
		g_sound_port = sound_port;
		return status;
	}
	return -1;
}

pj_status_t create_audio_stream(const pjmedia_codec_info* codec_info, pjmedia_dir media_dir,
		pj_uint16_t local_port, pj_sockaddr_in* remote_addr, pjmedia_stream** media_stream)//��ʼ����Ƶ��
{
	pjmedia_stream_info stream_info;//��Ƶ����Ϣ�����ڴ�����Ƶ��
	pjmedia_transport* media_transport = nullptr;// ý�崫�䣬���ڴ�����Ƶ��
	pj_status_t status;

	//��ʼ����Ƶ����Ϣ
	pj_bzero(&stream_info, sizeof(stream_info));
	//����仰�����ʣ�Ϊɶ��Ҫbzero������һ��ջ�ϵĽṹ�壬�ֲ��Ƕ��ϵ�
	//������֤������Ҫbzero����pjmedia_stream_create�������ж�stream_info.prama�Ƿ�Ϊnull�Ĳ���
	//�����Ϊnull���ͻ�������һ��memcpy���������prama������û�г�ʼ���ģ���0xccccccc��memcpy�ᱨ��
	stream_info.type = PJMEDIA_TYPE_AUDIO;
	stream_info.dir = media_dir;
	//�������codec_info��ֵ��stream_info�е�fmt��fmt����һ��codec_info��
	pj_memcpy(&stream_info.fmt, codec_info, sizeof(pjmedia_codec_info));
	stream_info.tx_pt = codec_info->pt;
	stream_info.rx_pt = codec_info->pt;
	stream_info.ssrc = pj_rand();
	//Զ�̵�ַ��ص���Ϣ��������Ƶ����Ϣ��
	pj_memcpy(&stream_info.rem_addr, remote_addr, sizeof(pj_sockaddr_in));
	status = pjmedia_transport_udp_create(media_endpt, nullptr, local_port, 0, &media_transport);
	if (status != PJ_SUCCESS);
	status = pjmedia_stream_create(media_endpt, pool, &stream_info, media_transport, nullptr, media_stream);
	if (status != PJ_SUCCESS);
	//��ʼ����
	pjmedia_transport_media_start(media_transport, nullptr, nullptr, nullptr, 0);
	return PJ_SUCCESS;
}
pj_status_t create_sound_port(pjmedia_stream* media_stream, pjmedia_snd_port** sound_port)//������Ƶ���˿�
{
	if (!media_stream || !sound_port)return -1;
	pj_status_t status;
	pjmedia_port* media_port = nullptr;//��Ƶ����Ϣ
	pjmedia_stream_info stream_info;//��Ƶ����Ϣ
	pjmedia_snd_port* temp_sound_port = nullptr;//��ʱ�����豸�˿�
	status = pjmedia_stream_get_port(media_stream, &media_port);
	if (status != PJ_SUCCESS)cout << "pjmedia_stream_get_port failed\n";;
	status = pjmedia_stream_get_info(media_stream, &stream_info);
	if (status != PJ_SUCCESS)cout << "pjmedia_stream_get_info failed\n";;
	// ������Ƶ�����򴴽������豸
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

	//���������豸����Ƶ��
	status = pjmedia_snd_port_connect(temp_sound_port, media_port);
	if (status != PJ_SUCCESS);
	*sound_port = temp_sound_port;
	return PJ_SUCCESS;

}
pj_status_t init()//����ʼ֮ǰ�ĳ�ʼ��
{
	pj_status_t status;
	status = pj_init();//��ʼ��pjlib,����д
	pj_caching_pool_init(&cache_pool, &pj_pool_factory_default_policy, 0);//�����ڴ�ع���(���Ǵ����ڴ��,�Ǵ��������ڴ�صĹ���)
	//todo ���0��max capacity������.
	if (status != PJ_SUCCESS)cout << "pj_init failed\n";
	pool = pj_pool_create(&cache_pool.factory, "PJSIP TEST", 4000, 4000, nullptr);
	status = pjmedia_event_mgr_create(pool, 0, nullptr);//����event manager������������Ϊ�����manager��ָ�룬û����ָ��ȥ����
	if (status != PJ_SUCCESS)cout << "pjmedia_event_mgr_create failed\n";
	status = pjmedia_endpt_create(&cache_pool.factory, nullptr, 1, &media_endpt);// ����ý��˵� ����1�������߳�����poll io
	if (status != PJ_SUCCESS)cout << "pjmedia_endpt_create failed\n";
	status = pjmedia_codec_register_audio_codecs(media_endpt, nullptr);// ע������֧�ֵ���Ƶ�������                                                                            c]
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
