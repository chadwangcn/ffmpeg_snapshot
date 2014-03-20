// example1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "jpeglib.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//定义一个压缩对象，这个对象用于处理主要的功能
	jpeg_compress_struct jpeg;
	//用于错误信息
	jpeg_error_mgr jerr;
	//错误输出在绑定
	jpeg.err = jpeg_std_error(&jerr);
 
	//初始化压缩对象
	jpeg_create_compress(&jpeg);
	//定义压缩后的输出，这里输出到一个文件！

#ifdef WIN32
	FILE* pFile = _wfopen( argv[1],L"wb" );
#else
	FILE* pFile = fopen( argv[1],"wb" );
#endif
	if( !pFile )
		return 0;
	//绑定输出
	jpeg_stdio_dest(&jpeg, pFile);
	//压缩参数设置。具体请到网上找相应的文档吧，参数很多，这里只设置主要的。
	//我设置为一个 24 位的 512　X　512大小的ＲＧＢ图片
	jpeg.image_width = 512;
	jpeg.image_height = 512;
	jpeg.input_components  = 3;
	jpeg.in_color_space = JCS_RGB;
	//参数设置为默认的
	jpeg_set_defaults(&jpeg);
	//还可以设置些其他参数：
	//// 指定亮度及色度质量
	//jpeg.q_scale_factor[0] = jpeg_quality_scaling(100);
	//jpeg.q_scale_factor[1] = jpeg_quality_scaling(100);
	//// 图像采样率，默认为2 * 2
	//jpeg.comp_info[0].v_samp_factor = 1;
	//jpeg.comp_info[0].h_samp_factor = 1;
	//// 设定编码jpeg压缩质量
	//jpeg_set_quality (&jpeg, 100, true);

	//开始压缩。执行这一行数据后，无法再设置参数了！
	jpeg_start_compress(&jpeg, TRUE);
	//定义一个数组，代表图片每一行的数据。3　代表　jpeg.input_components
	unsigned char oneRowImgData[ 512 * 3 ];
	for( int i=0;i<512*3;++i )
		oneRowImgData[i] = 0;
 
	JSAMPROW row_pointer[1];
	row_pointer[0] = oneRowImgData;
	//从上到下，设置图片中每一行的像素值
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

		//将一行数据写入！
		jpeg_write_scanlines( &jpeg,row_pointer,1 );
	}
	//结束压缩
	jpeg_finish_compress(&jpeg);
	//清空对象
	jpeg_destroy_compress(&jpeg);
 
	fclose( pFile ); pFile = NULL;
	return 0;
}

