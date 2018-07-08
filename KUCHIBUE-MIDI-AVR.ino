
#include "fix_fft2.h"
#include <avr/pgmspace.h>
#include <util/delay.h>
//#define DEBUG
#define THRESHOLD_LEVEL   25
#define ADC_STEP 64
extern char sqrt_dat[128][128];
extern char tone_data[3][41];
char im0[128], im1[128];
char data_ring[128], data0[128], data1[128];

char im0_lo[128], im1_lo[128];
char data_ring_lo[128], data0_lo[128], data1_lo[128];
char buf[41];

char midi_command = 0x90;
char midi_note = 0;
char midi_velo = 0;

char octave_offs = 0;

volatile int data_cnt = 0;
volatile int im_cnt = 0;
volatile uint8_t write_p = 0;
volatile uint8_t write_p_lo = 0;
volatile uint8_t read_p_lo = 0;


volatile char buf_no = 0;
volatile char lo_buf_no = 0;
char analized_buf_no = 0;
void send_tone(void);
char tone_no = 0;

char note_no[33] = {
  60, 60, //30, 31,
  61, 61, //32,  33,
  62, 62, //34,  35,
  63, 63, //36,  37,
  64, 64, //38,  39,
  65, 65, 65, //40,  41,  42,
  66, 66, 66, //43,  44,  45,
  67, 67, //46,  47,
  68, 68, //48,  49 ,
  69, 69, 69, //50,  51,  52,
  70, 70, 70, 70, //53,  54,  55,  56,
  71, 71, 71, //57,  58, 59,
  72, 72, 72, //60,  61,  62 ,

};

char note_no_lo[33] = {
  46,//27,
  47, 47, //28,  29,
  48, 48, //30,  32,
  49, 49, //33,  34,
  50, 50, //35,  36,
  51, 51, //37,  38,
  52, 52, //39,  40,
  53, 53, //41,  42,
  54, 54, //43,  44,
  55, 55, 55, //45,  46,  47,
  56, 56, 56, //48,  49,  50,
  57, 57, 57, //51,  52,  53,
  58, 58, 58, //54,  55,  56,
  59, 59, 59, //57,  58,  59,

};
void setup() {


  

  TIMSK0 = 0;
  ADCSRA = 0xF7;
  ADMUX = 0x60;
  DIDR0 = 0x01;

  /* Timer Setup */
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  OCR2A = 112;
  TCCR2A = 2;
  TCCR2B = 0x03;  // 1/32
  TIMSK2 = 2;
  DDRB = 0x07;
  PORTB = 0x07;
  
  _delay_ms(1000);
#ifdef DEBUG
  //Serial.begin(115200);  
   Serial.begin(256000);
#else
    if(!(PINB & 0x04) ){
      Serial.begin(115200); 
      
    }else{

      Serial.begin(2000000);      
    }
#endif





}

void loop() {
  int  i, j, k;
  char buf[6];
  char a;
  sei();
  send_tone(0);
  while (1) {


    while (analized_buf_no == buf_no){
      if(!(PINB & 0x01) ){
        tone_no++;
        if(tone_no > 2)
          tone_no = 0;
        send_tone(tone_no);
        _delay_ms(400);
      }
      if(!(PINB & 0x02) ){
        octave_offs = 12;
      }else{
        octave_offs = 0;
      }
      
      
    }
      

    analized_buf_no = buf_no;

    if (buf_no == 0) {

      fix_fft2(data0, im0); // full scale 2^7=128, FFT mode
      fix_fft2(data0_lo, im0_lo); // full scale 2^7=128, FFT mode
      for (i = 0; i <  ADC_STEP * 2 ; i++) {
        j = data0[i];
        k = im0[i] ;
        if (j < 0)
          j = -j;
        if (k < 0)
          k = -k;
        data0[i] = pgm_read_byte(&(sqrt_dat[j][k]));

        j = data0_lo[i];
        k = im0_lo[i] ;
        if (j < 0)
          j = -j;
        if (k < 0)
          k = -k;
        data0_lo[i] = pgm_read_byte(&(sqrt_dat[j][k]));
      }

#ifdef DEBUG
      dispMaxid("Max : ", data0, 63);
      dispMaxid("Max : ", data0_lo, 63);
      Serial.println("-");
#else
      sendCommand( data0, data0_lo, 63);
#endif

    } else {

      fix_fft2(data1, im1); // full scale 2^7=128 FFT mode
      fix_fft2(data1_lo, im1_lo); // full scale 2^7=128, FFT mode
      for (i = 0; i <  ADC_STEP * 2 ; i++) {

        j = data1[i];
        k = im1[i] ;
        if (j < 0)
          j = -j;
        if (k < 0)
          k = -k;
        data1[i] = pgm_read_byte(&(sqrt_dat[j][k]));

        j = data1_lo[i];
        k = im1_lo[i] ;
        if (j < 0)
          j = -j;
        if (k < 0)
          k = -k;
        data1_lo[i] = pgm_read_byte(&(sqrt_dat[j][k]));
      }

#ifdef DEBUG
      dispMaxid("Max : ", data1, 63);
      dispMaxid("Max : ", data1_lo, 63);
      Serial.println("+");
#else
      sendCommand( data1, data1_lo, 63);
#endif



    }

  }
}
void dispData(char *inMsg, char *inData, int inN) {
  Serial.println("");
  for (int i = 0; i < inN; i++) {
    sprintf(buf, "%2d", inData[i]);
    Serial.print(buf);
  }
}


void dispMaxid(char *inMsg, char *inData, int inN) {

  uint8_t max, max_id;
  int note;
  max = 0;
  max_id = 0;
  // for(int i = 1; i< inN;i++){
  for (int i = 12; i < inN; i++) {
    if (inData[i] > max) {
      max = inData[i];
      max_id = i;
    }

  }
  if (max > THRESHOLD_LEVEL) {
    sprintf(buf, "%3d  %3d   ", max_id, max);
    Serial.print(buf);
    //    Serial.println("");
  }
}


void sendCommand( char *inData, char *inData_lo, int inN) {

  uint8_t max, max_id;
  int note;
  max = 0;
  max_id = 0;

  for (int i = 4; i < 64; i++) {
    if (inData[i] > max) {
      max = inData[i];
      max_id = i;
    }

  }
  if (max_id < 30) {
    max_id = 0;
    max = 0;
    for (int i = 27; i < 60; i++) {
      if (inData_lo[i] > max) {
        max = inData_lo[i];
        max_id = i;
      }
    }
    if (max_id > 26 && max_id < 60) {
      note = note_no_lo[max_id-27]+ octave_offs;
    } else {
      max = 0;
    }
  } else {

    if (max_id > 29 && max_id < 63) {
      note = note_no[max_id-30] + octave_offs;
    } else {
      max = 0;
    }
  }









  if (max > THRESHOLD_LEVEL) {
    midi_command = 0x90;
    if (midi_velo == 0) {
      midi_velo = max+60 ;
      if(midi_velo > 127)
        midi_velo = 127;
      midi_note = note;

      send_midicommand(0x90, midi_note, midi_velo);

    } else {
      if (note != midi_note) {
        //   if(mnote[0] == note){
        send_midicommand(0x90, midi_note, 0);
        midi_velo = (max << 1) +30;
        if(midi_velo> 127)
          midi_velo = 127;
        midi_note = note;


        send_midicommand(0x90, midi_note, midi_velo);

      }

    }
  } else {
    if (max < THRESHOLD_LEVEL) {
      if (midi_command != 0) {
        if (midi_velo != 0) {
          send_midicommand(0x90, midi_note, 0);
          midi_command = 0;
          midi_velo = 0;
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
  ADCSRA = 0xD9;    //1/2 clock


}


ISR(ADC_vect) {
  char a;
  uint8_t read_p;
  a = ADCL;
  a = ADCH;

  ++write_p;
  write_p &= 0x7f;

  data_ring[write_p] = a;
  read_p = write_p + 64;
  read_p &= 0x7f;

  if (write_p & 0x01) {
    write_p_lo++;
    write_p_lo &= 0x7f;
    data_ring_lo[write_p_lo] = a;
  }

  if (buf_no == 0) {

    data1[data_cnt] = data_ring[read_p];
    data1[data_cnt + ADC_STEP] = a;


    read_p_lo++;
    read_p_lo &= 0x7f;

    data1_lo[data_cnt] = data_ring_lo[read_p_lo];
    read_p = read_p_lo + 64;
    read_p &= 0x7f;
    data1_lo[data_cnt + ADC_STEP] = data_ring_lo[read_p];



    im1[im_cnt] = 0;
    im1[im_cnt + ADC_STEP] = 0;
    im1_lo[im_cnt] = 0;
    im1_lo[im_cnt + ADC_STEP] = 0;

    im_cnt++;
    data_cnt++;
    if (data_cnt == ADC_STEP) {
      im_cnt = 0;
      data_cnt = 0;
      buf_no = 1;
      read_p_lo = write_p_lo + 32;
      read_p_lo &= 0x7f;

    }

  } else {

    data0[data_cnt] = data_ring[read_p];
    data0[data_cnt + ADC_STEP ] = a;

    read_p_lo++;
    read_p_lo &= 0x7f;
    data0_lo[data_cnt] = data_ring_lo[read_p_lo];
    read_p = read_p_lo + 64;
    read_p &= 0x7f;

    data0_lo[data_cnt + ADC_STEP] = data_ring_lo[read_p];



    im0[im_cnt] = 0;
    im0[im_cnt + ADC_STEP] = 0;
    im0_lo[im_cnt] = 0;
    im0_lo[im_cnt + ADC_STEP] = 0;

    im_cnt++;
    data_cnt++;
    if (data_cnt == ADC_STEP) {
      im_cnt = 0;
      data_cnt = 0;
      buf_no = 0;
      read_p_lo = write_p_lo + 32;
      read_p_lo &= 0x7f;

    }

  }

}

void send_tone(int no){
  buf[0] = 0xf0;
  buf[1] = 0x43;
  buf[2] = 0x7f;
  buf[3] = 0x02;
  buf[4] = 0x00;
  buf[5] = 0x00;
  buf[6] = 0x00;
  Serial.write(buf,7); // send header
  for(int i = 0;i<41;i++){
    buf[i] = pgm_read_byte(&(tone_data[no][i]));
  }
  Serial.write(buf,30);
  buf[0] = 0xf7;
  Serial.write(buf,1);
  
}


