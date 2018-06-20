
#include "fix_fft2.h"
#include <avr/pgmspace.h>


//#define THRESHOLD_LEVEL   10

#define THRESHOLD_LEVEL   12
//#define DEBUG
#define ADC_STEP   64

extern char sqrt_dat[128][128];
char im0[256], im1[256];
char data0[256], data1[256];
char buf[20];
int notes_cnt = 0;
char midi_command = 0x90;
char midi_note = 0;
char midi_velo = 0;
volatile int data_cnt = 0;
volatile int im_cnt = 0;
volatile char buf_no = 0;
char analized_buf_no = 0;



/* 
 *  FFT解析値toノートNo変換テーブル
 */
const char note_no[97] PROGMEM = {
  46, 46,   //27,28     3A#
  47, 47,
  48, 48,   //31,32     4C
  49, 49,
  50, 50,
  51, 51,
  52, 52,
  53, 53,
  54, 54,
  55, 55, 55,
  56, 56, 57,
  57, 57,
  58, 58, 58,
  59, 59, 59, 59,
  60, 60, 60, 61,
  61, 61, 61, 61,
  62, 62, 62,
  63, 63, 63, 63,
  64, 64, 64, 64, 64,
  65, 65, 65, 65,
  66, 66, 66, 66, 66, 66,
  67, 67, 67, 67, 67, 67,
  68, 68, 68, 68, 68, 68,
  69, 69, 69, 69, 69, 69,
  70, 70, 70,
  71, 71, 71, 71, 71, 71,
  72, 72, 72, 72, 72, 72, 72,

};
void setup() {

// Serial.begin(256000);
// Serial.begin(115200);

  Serial.begin(2000000);
  TIMSK0 = 0;
  ADCSRA = 0xF7;  
  ADMUX = 0x60;
  DIDR0 = 0x01;


  /* Timer Setup */
/* 
 *  サンプリングレート   452us  2.212KHz
 *  時間窓 452 * 256 =   115ms  8Hz
 *  1/4時間窓ごとに変換するので　32Hz周期で変換
 *  
 */
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
 
  OCR2A = 112;
  //OCR2A = 112;    //0x70
  TCCR2A = 2;
  TCCR2B = 0x03;  // 1/32
  TIMSK2 = 2;
  DDRB = 0x02;    //LED for DEBUG

}

void loop() {
  int  i, j, k;

  char a;
  sei();
  while (1) {

    // cli();


    while (analized_buf_no == buf_no)     //　サンプリング終了まで待機
      ;

    analized_buf_no = buf_no;
    
    if (buf_no == 0) {

      fix_fft2(data0, im0);  // full scale 2^8=256, FFT mode
      
      
      for (i = 0; i <  ADC_STEP * 2 ; i++) {

        j = data0[i];
        k = im0[i] ;
        if (j < 0)
          j = -j;
        if (k < 0)
          k = -k;
        data0[i] = pgm_read_byte(&(sqrt_dat[j][k]));
      }

#ifdef DEBUG
      dispMaxid("Max : ", data0, 127);
#else
      sendCommand("", data0, 127);
#endif

    } else {
      fix_fft2(data1, im1);  // full scale 2^8=256, FFT mode
      for (i = 0; i <  ADC_STEP * 2 ; i++) {

        j = data1[i];
        k = im1[i] ;
        if (j < 0)
          j = -j;
        if (k < 0)
          k = -k;
        data1[i] = pgm_read_byte(&(sqrt_dat[j][k]));
      }

#ifdef DEBUG
      dispMaxid("Max : ", data1, 127);
#else
      sendCommand("", data1, 127);
#endif
    }

  }
}

void dispMaxid(char *inMsg, char *inData, int inN) {

  char max, max_id;
  int note;
  max = 0;
  max_id = 0;
  // for(int i = 1; i< inN;i++){
  for (int i = 20; i < inN; i++) {
    if (inData[i] > max) {
      max = inData[i];
      max_id = i;
    }

  }
  if (max > THRESHOLD_LEVEL) {
    sprintf(buf, "%3d  %3d", max_id, max);
    Serial.println(buf);
  }else{
    sprintf(buf, "%3d  %3d", max_id, max);
     //Serial.println(buf);   
  }
 
}


void sendCommand(char *inMsg, char *inData, int inN) {

  char max, max_id;
  int note, i;
  max = 0;
  max_id = 0;

  // for(int i = 1; i< inN;i++){
  for (int i = 4; i < 128; i++) {
    if (inData[i] > max) {
      max = inData[i];
      max_id = i;
    }

  }

  if (max_id > 26 && max_id < 128) {
    note = pgm_read_byte(&(note_no[max_id - 27])) ;
  } else {
    max = 0;
  }
     PORTB = 0x02; 
  if (max > THRESHOLD_LEVEL) {

    midi_command = 0x90;
    if (midi_velo == 0) {
      midi_velo = max  + 80;
      midi_note = note;
      notes_cnt = 0;
      send_midicommand(0x90, midi_note, midi_velo);

    } else {
      if (note != midi_note) {
        if (notes_cnt > 0 ) {

          send_midicommand(0x90, midi_note, 0);
          midi_velo = max + 80;
          midi_note = note;
          notes_cnt = 0;

          send_midicommand(0x90, midi_note, midi_velo);

        }


      } else {
        if (notes_cnt < 20)
          notes_cnt++;
 //PORTB = 0x02;
      }

    }
  } else {
    if (max <= THRESHOLD_LEVEL) {
      if (midi_command != 0) {
        if (midi_velo != 0) {
          send_midicommand(0x90, midi_note, 0);
//PORTB = 0x00;
          midi_command = 0;
          midi_velo = 0;
          notes_cnt = 0;
        }
      }
    }
  }
}



void send_midicommand(char command, char note, char velo) {

  //  sprintf(buf,">>%03d,%03d,%03d",command,note,velo);
  //  Serial.print(buf);
  //    Serial.println("--");
  velo &= 0x7f;
  note &= 0x7f;
  buf[0] = 0x90;
  buf[1] = note;
  buf[2] = velo;

  Serial.write(buf, 3);
}

ISR(TIMER2_COMPA_vect) {
 // ADCSRA = 0xD9;    //1/2 clock
  ADCSRA = 0xD9;    //1/2 clock

}


ISR(ADC_vect) {
  char a;

  a = ADCL;
  a = ADCH;

  if (buf_no == 0) {
    data1[data_cnt] = data1[data_cnt + ADC_STEP * 2];
    data1[data_cnt + ADC_STEP] = data1[data_cnt + ADC_STEP * 3];
    data1[data_cnt + ADC_STEP * 2] = data0[data_cnt + ADC_STEP * 3];
    data1[data_cnt + ADC_STEP * 3] = a;

    im1[im_cnt] = 0;
    im1[im_cnt + ADC_STEP] = 0;
    im1[im_cnt + ADC_STEP * 2] = 0;
    im1[im_cnt + ADC_STEP * 3] = 0;
    im_cnt++;
    data_cnt++;
    if (data_cnt == ADC_STEP) {
      im_cnt = 0;
      data_cnt = 0;
      buf_no = 1;

    }

  } else {

    data0[data_cnt] = data0[data_cnt + ADC_STEP * 2];
    data0[data_cnt + ADC_STEP] = data0[data_cnt + ADC_STEP * 3];
    data0[data_cnt + ADC_STEP * 2] = data1[data_cnt + ADC_STEP * 3];
    data0[data_cnt + ADC_STEP * 3] = a;

    im0[im_cnt] = 0;
    im0[im_cnt + ADC_STEP] = 0;
    im0[im_cnt + ADC_STEP * 2] = 0;
    im0[im_cnt + ADC_STEP * 3] = 0;
    im_cnt++;
    data_cnt++;
    if (data_cnt == ADC_STEP) {
      im_cnt = 0;
      data_cnt = 0;
      buf_no = 0;
    }
  }

}
void send_tone(int no){
  
}






