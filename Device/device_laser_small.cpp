#include "device_laser_small.h"
//数字越大，二次采样频率越低
#define DOUBLE_SAMPLE_RATE 10
extern QByteArray tmpBuf;

//额外功能抽象接口
void device_laser_small::extern_fun(QString cmd){

}

void device_laser_small::start(){
    clear_readbuf();
    tmpBuf.clear();//清buf
}

//一次采集过程（小激光是极其高速的传感器，每秒发送数据可达1000条，避免计算机处理不过来，需要做二次采样）
//小激光不以\r\n表示一次数据结束，直接发送十六进制编码，需要特殊处理
//小激光用两个字节表示一次采集，例如0x84,0x71
//第一字节一定大于等于0x80,第二字节一定小于0x80
//直接结果为a = (0x84-0x80) * 0x80 + 0x71
//根据标定转换成真实的距离为 ret = (a - 40) * 200 / 4015 +60
void device_laser_small::sample(){
    //tmpBuf.append(IDevice::get_serial()->readAll());
    tmpBuf.append(receive_readbuf());
    //QByteArray get = IDevice::get_serial()->readAll();
    //处理buf
    int i = 0;
    int size = tmpBuf.count();
    if(size < 1) return;
    while(i < size){
        if(i >= 1 && (uchar)tmpBuf.at(i-1) >= 0x80 && (uchar)tmpBuf.at(i) < 0x80){
            uchar head = (uchar)tmpBuf.at(i-1);
            uchar tail = (uchar)tmpBuf.at(i);
            double tmp = (double)((head - 0x80) * 0x80 + tail);
            double result = (tmp - 40.0) * 200.0 / 4015.0 + 60;
            QByteArray str_bytes = QString::number(result,'f',3).toUtf8();
            QByteArray origin_bytes;
            origin_bytes.append(head);
            origin_bytes.append(tail);
            //emit IDevice::get_observer()->buf_show("HEX:"+origin_bytes.toHex());
            if(head >= 0x80 && head < 0xf0 && tail < 0x80){
                //正常范围
                emit IDevice::get_observer()->true_data_show(str_bytes);
            }else if(head >= 0xf0){
                //超过线性范围
                emit IDevice::get_observer()->true_data_show("OUT:"+str_bytes);
            }
            i+=2 * DOUBLE_SAMPLE_RATE;//后面的数表示二次采样的系数，数字越大，二次采样频率越低
            continue;
        }
        ++i;
    }
    tmpBuf.clear();
}

void device_laser_small::end(){

}
