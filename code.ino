/*
#############
#SST49LF020A#
#############
          Address   Data
Manufacturer’s ID   0000H     BFH
Device ID           0001H     52H
###########################################################
Device Access   Address Range         Memory Size
Memory Access   FFFF FFFFH : FFC0 0000H   4 MByte
Register Access FFBF FFFFH : FF80 0000H   4 MByte
###########################################################
~Addressing~
--------------------------------------------------------------------
The ID[3:0] bits in the address field are inverse of the hardware strapping. The ID[3:0] correspond to address bits [A21:A18]
--------------------------------------------------------------------
Device #    Hardware    Strapping Address Bits Decoding
      Strapping ID[3:0]   A21 A20 A19 A18
0       0000        1   1   1   1
1       0001        1   1   1   0
.
.
15      1111        0   0   0   0
###########################################################
   A31: A23   b=A22           A21:A18     A17:A0
1111 1111 1b  1=Mem Access  ID[3:0]   Device Memory address
              0=Reg Access


Connections
Arduino   PLCC
A0(PC0) - LAD0
A1(PC1) - LAD1
A2(PC2) - LAD2
A3(PC3) - LAD3
A4(PC4) - LFRAME
9 (PB1) - CLK



TIDA=150 ns

*/
//static long base_mem = 0x00000000;

//Pm49FL002
//static long base_mem = 0xFFFC0000;
//static long base_jdec =0xFFBC0000;
//static long size =0x40000;
//SST49LF020A
//static long base_mem = 0xFFC00000;
//static long base_jdec = 0xFF800000;
//static long size =0x40000;
//static long block_size=16 *1024;
//static long blocks=16; //number of blocks

//SST49LF040
static long base_mem = 0xFF800000;
static long base_jdec = 0xFF040000;
static long size =0x80000;
static long block_size=64 *1024;
static long blocks=8; //number of blocks
static long mem = 0x00;
char sdata[4];
int mdata=0x0;
int flag=0;
byte input=0x00;
byte data=0x00;
int data_out[16];
byte addr[4];

void set_lad_output(){DDRC |= 1<<PC0 | 1<<PC1 | 1<<PC2 | 1<<PC3;}
void set_lad_input(){DDRC &= 0<<PC0 | 0<<PC1 | 0<<PC2 | 0<<PC3 | 1<<PC4 | 1<<PC5;}
void set_lad_tristate(){wait_clock_neg();set_lad_input();  PORTC &=  0<<PC0 | 0<<PC1 | 0<<PC2 | 0<<PC3 | 1<<PC4 | 1<<PC5;}
void set_lad_data(byte data){wait_clock_neg();PORTC = (PINC & 0b110000) | data;}
byte read_lad(){
  input=0x0;
  wait_clock_neg();
  //wait_clock_pos();
  input=PINC&0x0F;
  return input;
}

void toggle_lframe(){
  set_lad_output();
  set_lad_data(0b0000);
  PORTC &= 0b101111; //0<<PC4
  wait_clock_neg();
  PORTC |= 1<<PC4;
  flag=1;
}

void wait_clock_neg(){
  while(!flag){__asm__ __volatile__ ("nop\n\t");}
  flag=0;
  }

void wait_clock_pos(){
  while(flag){__asm__ __volatile__ ("nop\n\t");}
  }

ISR(TIMER1_COMPA_vect){
  if(!(PINB & 1<< PB1))
    {flag=1;}
  else
    {flag=0;}
  }


void setup() {
    delay(600);
    cli();
    Serial.begin(115200);
    Serial.println("Starting program");
    
    TCCR1A = 0<<COM1A1 | 1<<COM1A0 | 0<<WGM01 | 0<<WGM11;//Toggle OC1A on compare match
    TCNT1H=0x0; //clear counter
    TCNT1L=0x0; //clear counter
    OCR1AL =20; //set interrupt at 1mhz
    OCR1AH=0;
    TIMSK1|= 1<<OCIE1A;//enable interrupts from Timer/Counter Compare Match A
    TCCR1B|= 1<<WGM12 | 0<<CS12 | 0<<CS11 | 1<<CS11;// No Prescaler and start counting
    DDRC |= 1<<PC5; //flash reset pin high
    DDRC |= 1<<PC4;//LFrame pin output
    PORTC |= 1<<PC0 | 1<<PC1 | 1<<PC2 | 1<<PC3 | 1<<PC4 | 1<<PC5;
    DDRB |= 1<<PB1; //clock (OC1A) pin output
    sei();
    //delay(1200);
    //chip_erase();
    
    //read_chip_id();
    soft_id();
    //block_erase(1);
    //block_erase(2);
    read_flash();
    //Serial.println(read_address(size-10));
    

}
void loop(){
  //Serial.println(PIND & 1<< PD6,BIN);
  }

void read_flash(){
    unsigned long i=0;
    while(i<=size){
      if(i % 16==0 && i>0){
        cli();
        for(byte j=0;j<=15;j++){
          sprintf(sdata,"%02x ",data_out[j]);
          Serial.print(sdata);
        }
        Serial.println();
        sei();
     }
      data_out[(i % 16)]=read_address(i);
      i++;
    }
  }

void read_chip_id(){
    mem=base_jdec;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR
    for(byte i=1;i<=8;i++){
    set_lad_data(mem>>32-4*i);}
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x0){Serial.println("Error In Sync at Cycle 13"); }//Sync
    data=read_lad();//Data ls nibble
    data|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){Serial.println("Error In Sync at Cycle 16"); }//Sync
    set_lad_output();
    set_lad_data(0b1111);//TAR0
    Serial.print("Manufacturer ID: ");
    Serial.println(data,HEX);

    mem=base_jdec+1;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR
    for(byte i=1;i<=8;i++){
    set_lad_data(mem>>32-4*i);}
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x0){Serial.println("Error In Sync at Cycle 13"); }//Sync
    data=read_lad();//Data ls nibble
    data|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){Serial.println("Error In Sync at Cycle 16"); }//Sync
    set_lad_output();
    set_lad_data(0b1111);//TAR0
    Serial.print("Device ID: ");
    Serial.println(data,HEX);  
  }
  
byte read_address(unsigned long address){
    byte rdata=0x00;
    address=address+base_mem;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR
    for(byte i=1;i<=8;i++){
    set_lad_data(address>>32-4*i);}
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x0){Serial.println("Error In Sync at Cycle 13"); }//Sync
    rdata=read_lad();//Data ls nibble
    rdata|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){Serial.println("Error In Sync at Cycle 16"); }//Sync
    return rdata;  
  }
bool toggle_d6(){
  byte temp=0x0;
  delay(20);
  byte cnt=0;
  while(cnt++<100){
      temp=read_address(base_mem) & 0x40;
      data=read_address(base_mem) & 0x40;
      if(temp==data){return 0;}
    }return 1;
  }
    
int write_address(unsigned long address,  byte wdata){
    address=base_mem+address;
    toggle_lframe();
    set_lad_data(0b0111);//CYCTYPE + DIR write command
    for(byte i=1;i<=8;i++){
    set_lad_data(address>>32-4*i);}
    
    set_lad_data(wdata);//ls nibble first
    set_lad_data(wdata>>4);//ms nibble last
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x00){Serial.println("Error In Sync at WCycle 13");}//Sync
    if(read_lad()!=0x0F){Serial.println("Error In Sync at WCycle 16");}//Sync
    //set_lad_tristate();//TAR1 
    set_lad_output();
    set_lad_data(0b1111);//TAR0
     
  }
  
void soft_id(){
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0x90);
    data=read_address(0x0000);
    Serial.print("Manufacturer ID: ");
    Serial.println(data,HEX);  
    data=read_address(0x0001);
    Serial.print("Device ID: ");
    Serial.println(data,HEX);
    write_address(0x5555, 0xF0);
    if(toggle_d6()) Serial.println("Unknown Error Occured");
    }
    
void block_erase(byte block){
    unsigned long block_addr=block*block_size;//calculation of block base address(block_address=block_number*block_size)
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0x80);
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(block_addr, 0x50);
    //while(1){Serial.println(read_address(block_addr),HEX);}
    //while(!(read_address(block_addr) & 0x80)){Serial.println("lol");}
    //data=read_address(block_addr) & 0x40;
    //while(data!=0x80){data=read_address(block_addr) & 0x80;}
    if(!toggle_d6()){
    Serial.print("Block ");Serial.print(block);Serial.println(" Erased ");}
    else Serial.println("Unknown Error Occured");
    //delay(10);
  }
void chip_erase(){
  for(byte i=0;i<blocks;i++){
    block_erase(i);
    }
  
}

void program_address(unsigned long address, byte bdata){
    //block=(block<<12);
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0xA0);
    write_address(address, bdata);
    if(toggle_d6()) Serial.println("Unknown Error Occured");
    else Serial.println("Address Programed ");
  }

/* -for debugging purposes only-
byte read_address1(unsigned long address){
  data=0x00;
  //long address to 4 byte array
  for(byte i=0;i<=3;i++){
    addr[i]=address>>i*8;
  }
  toggle_lframe();
  //Serial.println(PINC,BIN);
  set_lad_data(0b0100);//CYCTYPE + DIR
  //Serial.println(PINC,BIN);
  set_lad_data(0b1111);//A[31:28]
  //Serial.println(PINC,BIN);
  set_lad_data(0b1111);//A[27:24]
  //Serial.println(PINC,BIN);
  set_lad_data(0b1100);//A[23:20], A22=1=Mem Access, A21:A20=ID[3:2]=00
  //Serial.println(PINC,BIN);
  set_lad_data(0b0000 | addr[2]);///A[19:16], A[19:18]=ID[1:0]=00, A[17:16]=2 lsb addr[2]
  //Serial.println(PINC,BIN);
  set_lad_data(addr[1]>>4);//A[15:12]
  //Serial.println(PINC,BIN);
  set_lad_data(addr[1] & 0x0F);//A[11:8]
  //Serial.println(PINC,BIN);
  set_lad_data(addr[0]>>4);//A[7:4]
  //Serial.println(PINC,BIN);
  set_lad_data(addr[0] & 0x0F);//A[3:0]
  //Serial.println(PINC,BIN);
  set_lad_data(0b1111);//TAR0
  //Serial.println(PINC,BIN);
  set_lad_tristate();//TAR1
  //Serial.println(PINC,BIN);
  if(read_lad()!=0x0){Serial.println("Error In Sync at Cycle 13"); }//Sync
  data=read_lad();//Data ls nibble
  data|=read_lad()<<4;//Data ms nibble
  if(read_lad()!=0x0F){Serial.println("Error In Sync at Cycle 16"); }//Sync
  return data;
}
*/
