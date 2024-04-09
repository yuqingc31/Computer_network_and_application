#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "emulator.h"
#include "altbit.h"

/* ******************************************************************
   Unfinished Alternating bit protocol.  Adapted from
   ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   Network properties:
   - one way network delay averages five time units (longer if there
   are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
   or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
   (although some can be lost).
   网络属性：
   - 单程网络延迟平均为五个时间单位（如果通道中存在其他消息，则延迟更长），但可能更长
   - 数据包可能会损坏（标题或数据部分）或根据用户定义的概率丢失
   - 数据包将按发送顺序传递
   (尽管可能会丢失一些)。


   Modifications (6/6/2008 - CLP): 
   - removed bidirectional code and other code not used by prac. 
   - fixed C style to adhere to current programming style
   (7/8/2009 - CLP)
   - converted to Alt Bit

   修改（2008年6月6日 - CLP）：
   - 删除了双向代码和实践中未使用的其他代码。
   - 修复了C样式，以符合当前的编程风格
   (2009年7月8日 - CLP)
   - 转换为Alt Bit
**********************************************************************/

#define RTT  15.0       /* round trip time.  MUST BE SET TO 15.0 when submitting assignment */ /* 往返时间。在提交任务时必须设置为15.0 */
#define WINDOWSIZE 1    /* alternating bit only allows one unacked packet */ /* 交替比特仅允许一个未确认的数据包 */
#define NOTINUSE (-1)   /* used to fill header fields that are not being used */ /* 用于填充未使用的标题字段 */

/* generic procedure to compute the checksum of a packet.  Used by both sender and receiver  
   the simulator will overwrite part of your packet with 'z's.  It will not overwrite your 
   original checksum.  This procedure must generate a different checksum to the original if
   the packet is corrupted.
*/
/* 用于计算数据包校验和的通用过程。被发送方和接收方使用
   模拟器将使用“z”覆盖您数据包的一部分。它不会覆盖您的
   原始校验和。如果数据包损坏，则此过程必须生成与原始校验和不同的校验和。
*/
int ComputeChecksum(struct pkt packet)
{
  int checksum = 0;
  int i;
  /****** 4. FILL IN CODE to calculate the checksum of packet *****/
  /****** 4. 填写代码以计算数据包的校验和 *****/

  /****** add the seqnum and acknum to the checksum ******/
  checksum += packet.seqnum;
  /****** add the acknum to the checksum ******/
  checksum += packet.acknum;
  /****** add the payload to the checksum *****/
  for (i = 0; i < 20; i++)
  {
    checksum += packet.payload[i];
  }
  return checksum;
}

bool IsCorrupted(struct pkt packet)
{
  if (packet.checksum == ComputeChecksum(packet))
    return (false);
  else
    return (true);
}

/********* Sender (A) variables and functions ************/
/********* 发送方（A）变量和函数 ************/

static struct pkt buffer[WINDOWSIZE];  /* array for storing packets waiting for ACK */ /* 存储等待ACK的数据包的数组 */
static int windowfirst, windowlast;    /* array indexes of the first/last packet awaiting ACK */ /* 等待ACK的第一个/最后一个数据包的数组索引 */
static int windowcount;                /* the number of packets currently awaiting an ACK */ /* 当前等待ACK的数据包数量 */
static int A_nextseqnum;               /* the next sequence number to be used by the sender */ /* 发送方将要使用的下一个序列号 */

/* called from layer 5 (application layer), passed the message to be sent to other side */
/* 从层5（应用层）调用，传递要发送到另一侧的消息 */
void A_output(struct msg message)
{
  struct pkt sendpkt;
  int i;

  /* if not blocked waiting on ACK */
  /* 如果没有被阻塞等待ACK */
  if ( windowcount < WINDOWSIZE) {
    if (TRACE > 1)
      printf("----A: New message arrives, send window is not full, send new messge to layer3!\n");

    /* create packet */
    /* 创建数据包 */
    sendpkt.seqnum = A_nextseqnum;
    sendpkt.acknum = NOTINUSE;
    for ( i=0; i<20 ; i++ ) 
      sendpkt.payload[i] = message.data[i];
    sendpkt.checksum = ComputeChecksum(sendpkt); 

    /* put packet in window buffer */
    /* windowlast will always be 0 for alternating bit; but not for GoBackN */
    /* 将数据包放入窗口缓冲区 */
    /* 对于交替比特，windowlast将始终为0；但对于GoBackN不是 */
    windowlast = (windowlast + 1) % WINDOWSIZE; 
    buffer[windowlast] = sendpkt;
    for (i=0; i<20; i++)
      buffer[windowlast].payload[i]=sendpkt.payload[i];  /* copy the array */
    windowcount++;

    /* send out packet */
    if (TRACE > 0)
      printf("Sending packet %d to layer 3\n", sendpkt.seqnum);
    tolayer3 (A, sendpkt);
    /**** 1. FILL IN CODE There's something else A needs to do when it sends a packet. *****/
    /**** 1. 填写代码 A 发送数据包时需要做的事情 *****/
    /**** if this is the first packet in the window, start the timer ******/
    starttimer(A,RTT);

    A_nextseqnum = (A_nextseqnum + 1) % 2;  /* we only have seqnum 0 and 1 */ /* 我们只有序列号0和1 */
  }
  /* if blocked,  window is full */
  else {
    if (TRACE > 0)
      printf("----A: New message arrives, send window is full\n");
    window_full++;
  }
}


/* called from layer 3, when a packet arrives for layer 4 
   In this practical this will always be an ACK as B never sends data.
*/
/* 从层3调用，当数据包到达层4时（在这个练习中，这将始终是ACK，因为B永远不会发送数据）。 */
void A_input(struct pkt packet)
{

  /* if received ACK is not corrupted */ 
  /* 如果接收到的ACK未损坏 */
  if (!IsCorrupted(packet)) {
    if (TRACE > 0)
      printf("----A: uncorrupted ACK %d is received\n",packet.acknum);
    total_ACKs_received++;

    /* check if new ACK or duplicate */
    /* 填写代码： 检查是否是新的ACK还是重复的ACK */
    if (windowcount >= WINDOWSIZE && packet.acknum != A_nextseqnum) {

      /* 数据包是新的ACK */
      if (TRACE > 0)
        printf("----A: ACK %d is not a duplicate\n",packet.acknum);
      new_ACKs++;

      /* delete the acked packets from window buffer */
      /* 从窗口缓冲区删除已确认的数据包 */
      windowcount--;

      /***** 1. FILL IN CODE  What else needs to be done when an ACK arrives
       besides removing the packet from the window?  ****/
       /***** 1. 填写代码 当ACK到达时还需要做些什么
       除了从窗口中删除数据包之外？ ****/

      /* if this was the last packet in the window, stop the timer */
      stoptimer(A);
    }
    else
      if (TRACE > 0)
        printf ("----A: duplicate ACK received, do nothing!\n");
  }
  else 
    if (TRACE > 0)
      printf ("----A: corrupted ACK is received, do nothing!\n");
}

/* called when A's timer goes off */
/* 当A的定时器超时时调用 */
void A_timerinterrupt(void)
{

  if (TRACE > 0)
    printf("----A: time out,resend packets!\n");

  if (TRACE > 0)
    printf ("---A: resending packet %d\n", (buffer[windowfirst]).seqnum);
  tolayer3(A,buffer[windowfirst]);
  /**** 1. FILL IN CODE What state should the timer be in at this point? *****/
  /****** 1. 填写代码 此时定时器应处于什么状态 ****/

  starttimer(A, RTT);
  packets_resent++;
}       



/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
/* 这个函数将在调用任何其他实体A函数之前调用一次（仅一次）。您可以使用它来进行任何初始化 */
void A_init(void)
{
  /* initialise A's window, buffer and sequence number */
   /* 初始化A的窗口、缓冲区和序列号 */
  A_nextseqnum = 0;  /* A starts with seq num 0, do not change this */ /* A从序列号0开始，不要更改此值 */
  windowfirst = 0;
  windowlast = -1;   /* windowlast is where the last packet sent is stored.  
		     new packets are placed in winlast + 1 
		     so initially this is set to -1		   */
         /* windowlast是存储最后一个发送的数据包的地方。
        新的数据包放置在winlast + 1
        所以最初将其设置为-1 */
  windowcount = 0;
}



/********* Receiver (B)  variables and procedures ************/

/********* 接收方（B）变量和过程 ************/

static int expectedseqnum; /* the sequence number expected next by the receiver */ /* 接收方期望的下一个序列号 */
static int B_nextseqnum;   /* the sequence number for the next packets sent by B */ /* B发送的下一个数据包的序列号 */


/* called from layer 3, when a packet arrives for layer 4 at B*/
/* 从层3调用，当数据包到达B层4时 */
void B_input(struct pkt packet)
{
  struct pkt sendpkt;
  int i;

  /* if not corrupted and received packet is in order */
  /* 如果未损坏且接收到的数据包顺序正确 */
  if  ( (!IsCorrupted(packet))  && (packet.seqnum == expectedseqnum) ) {
    if (TRACE > 0)
      printf("----B: packet %d is correctly received, send ACK!\n",packet.seqnum);
    packets_received++;

    /* deliver to receiving application */
    /* 传递给接收应用程序 */
    tolayer5(B, packet.payload);

    /* send an ACK for the received packet */
    /* 为接收到的数据包发送ACK */
    sendpkt.acknum = expectedseqnum;

    /* update state variables */
    /* 更新状态变量 */
    expectedseqnum = (expectedseqnum + 1) % 2;        
  }
  else {
    /* packet is corrupted or out of order */
    /* 数据包损坏或顺序错误 */
    if (TRACE > 0) 
      printf("----B: packet corrupted or not expected sequence number, resend ACK!\n");
    /***** 3. FILL IN CODE  What ACK number should be sent if the packet
	   was corrupted or out of order? *******/ 
     /***** 3. 填写代码 如果数据包损坏或顺序错误，则应发送什么ACK编号？ *******/

    /*****  Because the receiver cannot process the current packet correctly, it needs to inform the sender of the sequence number it expects for the next valid packet. *****/
    sendpkt.acknum = (expectedseqnum + 1) % 2;
  }

  /* create packet */
  /* 创建数据包 */
  sendpkt.seqnum = B_nextseqnum;
  B_nextseqnum = (B_nextseqnum + 1) % 2;
    
  /* we don't have any data to send.  fill payload with 0's */
  /* 我们没有要发送的数据。用0填充有效载荷 */
  for ( i=0; i<20 ; i++ ) 
    sendpkt.payload[i] = '0';  

  /* computer checksum */
  /* 计算校验和 */
  sendpkt.checksum = ComputeChecksum(sendpkt); 

  /* send out packet */
  /* 发送数据包 */
  tolayer3 (B, sendpkt);
}

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
/* 这个函数将在调用任何其他实体B函数之前调用一次（仅一次）。您可以使用它来进行任何初始化 */
void B_init(void)
{
  expectedseqnum = 0;
  B_nextseqnum = 1;
}

/******************************************************************************
 * The following functions need be completed only for bi-directional messages *
 * They do not need to be completed for this practical                        *
 *****************************************************************************/

/* Note that with simplex transfer from a-to-B, there is no B_output() */
/******************************************************************************
 * 下面的函数仅需要为双向消息完成
 * 对于此练习，它们不需要完成
 *****************************************************************************/

/* 请注意，对于从A到B的单向传输，没有B_output() */
void B_output(struct msg message)  
{
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
}