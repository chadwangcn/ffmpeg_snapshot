// example1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jpeglib.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//����һ��ѹ����������������ڴ�����Ҫ�Ĺ���
	jpeg_compress_struct jpeg;
	//���ڴ�����Ϣ
	jpeg_error_mgr jerr;
	//��������ڰ�
	jpeg.err = jpeg_std_error(&jerr);
 
	//��ʼ��ѹ������
	jpeg_create_compress(&jpeg);
	//����ѹ�������������������һ���ļ���

#ifdef WIN32
	FILE* pFile = _wfopen( argv[1],L"wb" );
#else
	FILE* pFile = fopen( argv[1],"wb" );
#endif
	if( !pFile )
		return 0;
	//�����
	jpeg_stdio_dest(&jpeg, pFile);
	//ѹ���������á������뵽��������Ӧ���ĵ��ɣ������ܶ࣬����ֻ������Ҫ�ġ�
	//������Ϊһ�� 24 λ�� 512��X��512��С�ģңǣ�ͼƬ
	jpeg.image_width = 512;
	jpeg.image_height = 512;
	jpeg.input_components  = 3;
	jpeg.in_color_space = JCS_RGB;
	//��������ΪĬ�ϵ�
	jpeg_set_defaults(&jpeg);
	//����������Щ����������
	//// ָ�����ȼ�ɫ������
	//jpeg.q_scale_factor[0] = jpeg_quality_scaling(100);
	//jpeg.q_scale_factor[1] = jpeg_quality_scaling(100);
	//// ͼ������ʣ�Ĭ��Ϊ2 * 2
	//jpeg.comp_info[0].v_samp_factor = 1;
	//jpeg.comp_info[0].h_samp_factor = 1;
	//// �趨����jpegѹ������
	//jpeg_set_quality (&jpeg, 100, true);

	//��ʼѹ����ִ����һ�����ݺ��޷������ò����ˣ�
	jpeg_start_compress(&jpeg, TRUE);
	//����һ�����飬����ͼƬÿһ�е����ݡ�3������jpeg.input_components
	unsigned char oneRowImgData[ 512 * 3 ];
	for( int i=0;i<512*3;++i )
		oneRowImgData[i] = 0;
 
	JSAMPROW row_pointer[1];
	row_pointer[0] = oneRowImgData;
	//���ϵ��£�����ͼƬ��ÿһ�е�����ֵ
	for( int i=0;i<jpeg.image_height;++i )
	{
		int data = 0;
		for (int j = 0;j<jpeg.image_width*3;j=j+3)
		{
			oneRowImgData[j] = data%255;//R
			//oneRowImgData[j+1] = data%255;//G
			//oneRowImgData[j+2] = data%255;//B
			data ++;
		}

		//��һ������д�룡
		jpeg_write_scanlines( &jpeg,row_pointer,1 );
	}
	//����ѹ��
	jpeg_finish_compress(&jpeg);
	//��ն���
	jpeg_destroy_compress(&jpeg);
 
	fclose( pFile ); pFile = NULL;
	return 0;
}

