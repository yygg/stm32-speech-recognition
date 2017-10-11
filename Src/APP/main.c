/********* main.C **********/

#include "includes.h"
#include "VAD.H"
#include "MFCC.H"
#include "DTW.H"
#include "GUI.H"
#include "flash.h"
#include "delay.h"

#define DEBUG
#ifdef DEBUG
#define DBP(fmt,arg...)  USART1_printf(fmt,##arg)
#define DBPLN(fmt,arg...)  USART1_printf_line(fmt,##arg)
#define DBPH(src, len)  USART1_print_hex(src, len)
#else
#define DBP(fmt,arg...)
#define DBPLN(fmt,arg...)
#define DBPH(src, len)
#endif

u16 		VcBuf[VcBuf_Len];
atap_tag	atap_arg;
valid_tag	valid_voice[max_vc_con];
v_ftr_tag	ftr;
typedef struct
{
	u8 str[3];
}comm_tag;

comm_tag commstr[]={"0 ","1 ","2 ","3 ","4 ","5 ","6 ","7 ","8 ","9 ","��","��","ǰ","��","��","��","��","С"};

#define sel_clor		BRED
#define dis_sel_clor	GRED
#define spk_clor		BRED
#define prc_clor		GRED

#define save_ok		0
#define VAD_fail	1
#define MFCC_fail	2
#define Flash_fail	3

void disp_comm(u8 comm)
{
	GUI_ClrArea(&(Label[comm]));
	GUI_DispStr(&(Label[comm]),(u8 *)(commstr[comm-G_comm_fst].str));
}

void set_comm_backclor(u8 comm, u16 backclor)
{
	Label[comm].BackColor=backclor;
	disp_comm(comm);
}

void set_label_backclor(GUI_Area *Label, u16 backclor)
{
	Label->BackColor=backclor;
	GUI_ClrArea(Label);
}

void disp_home(void)
{
	GUI_ClrArea(&Screen);
	GUI_ClrArea(&(Label[G_wel]));
	GUI_DispStr(&(Label[G_wel]),"��ӭʹ��");
	GUI_ClrArea(&(Label[G_neme]));
	GUI_DispStr(&(Label[G_neme]),"����������ʶ�����ϵͳ");
	GUI_ClrArea(&(Label[G_prc]));
	GUI_DispStr(&(Label[G_prc]),"ģ��ѵ��");
	GUI_ClrArea(&(Label[G_recg]));
	GUI_DispStr(&(Label[G_recg]),"����ʶ��");
	GUI_ClrArea(&(Label[G_designer]));
	GUI_DispStr(&(Label[G_designer]),"�����:�ν�");
	
}

void record(void)
{
	delay_ms(atap_len_t);	//��ʱ����������Ļ����������
	
	TIM_Cmd(TIM1, ENABLE); 	//������ʱ������ʼ�źŲɼ�
	
	GUI_ClrArea(&(Label[G_ctrl]));					//��ʾ������ʾ
	GUI_DispStr(&(Label[G_ctrl]),"¼����");		
	
	//��ʼ˵��֮ǰ��¼��һС�α�������������ʵ�ֱ�����������Ӧ
	delay_ms(atap_len_t);		
	//��ʾ��ʼ˵��
	set_label_backclor(&(Label[G_spk]), spk_clor);
	
	//�ȴ����������ݸ������
	while(DMA_GetFlagStatus(DMA1_FLAG_TC1)==RESET);
	
	
	//���ݲɼ��������رն�ʱ��
	TIM_Cmd(TIM1, DISABLE); 
	//�����ݴ�����ɱ�־���Ա��´�ʹ��
	DMA_ClearFlag(DMA1_FLAG_TC1);
	
	//��ʾ��ʼ����ɼ���������
	set_label_backclor(&(Label[G_spk]), prc_clor);
}

void disp_mdl_prc(void)
{
	u16 i;
	
	GUI_ClrArea(&Screen);
	
	set_label_backclor(&(Label[G_cap]), BRED);
	GUI_DispStr(&(Label[G_cap]),"��ʼѵ��");
	
	for(i=G_comm_fst;i<=G_comm_lst;i++)
	{
		disp_comm(i);
	}
	GUI_ClrArea(&(Label[G_return]));
	GUI_DispStr(&(Label[G_return]),"����");
}

u8 save_mdl(u16 *v_dat, u32 addr)
{
	noise_atap(v_dat,atap_len,&atap_arg);
	
	VAD(v_dat, VcBuf_Len, valid_voice, &atap_arg);
	if(valid_voice[0].end==((void *)0))
	{
		return VAD_fail;
	}
	
	get_mfcc(&(valid_voice[0]),&ftr,&atap_arg);
	if(ftr.frm_num==0)
	{
		return MFCC_fail;
	}
	
	return save_ftr_mdl(&ftr, addr);
}

void prc(void)
{
	u32 i;
	u8	prc_start=0;
	u8	comm=G_comm_fst;
	u8 	prc_count=0;
	u32 addr;
	//v_ftr_tag *sav_ftr;
	
	disp_mdl_prc();
	set_comm_backclor(comm,sel_clor);
	
	while(1)
	{
		wait_touch();
		if(touch_area(&(Label[G_return])))
		{
			Label[G_cap].BackColor=GREEN;
			Label[comm].BackColor=dis_sel_clor;
			disp_home();
			return;
		}
		else if(touch_area(&(Label[G_cap])))
		{
			delay_ms(150);
			if(prc_start==0)
			{
				GUI_ClrArea(&(Label[G_cap]));
				GUI_DispStr(&(Label[G_cap]),"ֹͣѵ��");
				prc_start=1;
				GUI_ClrArea(&(Label[G_ctrl]));
				GUI_DispStr(&(Label[G_ctrl]),"��ʼ");
				GUI_ClrArea(&(Label[G_spk]));
				
				GUI_ClrArea(&(Label[G_count]));
				GUI_DispStr(&(Label[G_count]),"��ѵ��0��");
			}
			else
			{
				GUI_ClrArea(&(Label[G_cap]));
				GUI_DispStr(&(Label[G_cap]),"��ʼѵ��");
				prc_start=0;
				prc_count=0;
				
				GUI_HideArea(&(Label[G_ctrl]));
				GUI_HideArea(&(Label[G_spk]));
				GUI_HideArea(&(Label[G_stus]));
				GUI_HideArea(&(Label[G_count]));
				
			}
		}
		else if((touch_area(&(Label[G_ctrl])))&&(prc_start==1))
		{
			record();
			
			GUI_ClrArea(&(Label[G_ctrl]));
			GUI_DispStr(&(Label[G_ctrl]),"��ȡ��");
			
			addr=ftr_start_addr+(comm-G_comm_fst)*size_per_comm+prc_count*size_per_ftr;
			if(save_mdl(VcBuf, addr)==save_ok)
			{
				prc_count++;
				GUI_ClrArea(&(Label[G_count]));
				GUI_printf(&(Label[G_count]),"��ѵ��%d��",prc_count);
				if(prc_count==ftr_per_comm)
				{
					prc_count=0;
				}
				GUI_ClrArea(&(Label[G_stus]));
				GUI_DispStr(&(Label[G_stus]),"������Ч");
				/*
				sav_ftr=(v_ftr_tag *)addr;
				USART1_printf("mask=%d ",sav_ftr->save_sign);
				USART1_printf("frm_num=%d",sav_ftr->frm_num);
				for(i=0;i<((sav_ftr->frm_num)*mfcc_num);i++)
				{
					USART1_printf("%d,",sav_ftr->mfcc_dat[i]);
				}
				*/
			}
			else
			{
				GUI_ClrArea(&(Label[G_stus]));
				GUI_DispStr(&(Label[G_stus]),"������Ч");
			}

			GUI_ClrArea(&(Label[G_ctrl]));
			GUI_DispStr(&(Label[G_ctrl]),"��ʼ");
		}
		else if(prc_start==0)
		{
			for(i=G_comm_fst;i<=G_comm_lst;i++)
			{
				if(touch_area(&(Label[i])))
				{
					set_comm_backclor(comm,dis_sel_clor);
					comm=i;
					set_comm_backclor(comm,sel_clor);
					break;
				}
			}
		}
		
	}
}




u8* spch_recg(u16 *v_dat, u32 *mtch_dis)
{
	u16 i;
	u32 ftr_addr;
	u32 min_dis;
	u16 min_comm;
	u32 cur_dis;
	v_ftr_tag *ftr_mdl;
	
	noise_atap(v_dat, atap_len, &atap_arg);
	
	VAD(v_dat, VcBuf_Len, valid_voice, &atap_arg);
	if(valid_voice[0].end==((void *)0))
	{
		*mtch_dis=dis_err;
		USART1_printf("VAD fail ");
		return (void *)0;
	}
	
	get_mfcc(&(valid_voice[0]),&ftr,&atap_arg);
	if(ftr.frm_num==0)
	{
		*mtch_dis=dis_err;
		USART1_printf("MFCC fail ");
		return (void *)0;
	}
	
	i=0;
	min_comm=0;
	min_dis=dis_max;
	for(ftr_addr=ftr_start_addr; ftr_addr<ftr_end_addr; ftr_addr+=size_per_ftr)
	{
		ftr_mdl=(v_ftr_tag*)ftr_addr;
		//USART1_printf("save_mask=%d ",ftr_mdl->save_sign);
		cur_dis=((ftr_mdl->save_sign)==save_mask)?dtw(&ftr,ftr_mdl):dis_err;
		//USART1_printf("cur_dis=%d ",cur_dis);
		if(cur_dis<min_dis)
		{
			min_dis=cur_dis;
			min_comm=i;
		}
		i++;
	}
	min_comm/=ftr_per_comm;
	//USART1_printf("recg end ");
	*mtch_dis=min_dis;
	return (commstr[min_comm].str);
}

void disp_recg(void)
{
	GUI_ClrArea(&Screen);
	GUI_ClrArea(&(Label[G_cap]));
	GUI_DispStr(&(Label[G_cap]),"����ʶ��");
	
	GUI_ClrArea(&(Label[G_ctrl]));
	GUI_DispStr(&(Label[G_ctrl]),"��ʼ");
	GUI_ClrArea(&(Label[G_spk]));
	
	GUI_ClrArea(&(Label[G_return]));
	GUI_DispStr(&(Label[G_return]),"����");
}


void recg(void)
{
	u8 *res;
	u32 dis;
	u32 recg_count=0;
	
	disp_recg();
	
	while(1)
	{
		wait_touch();
		if(touch_area(&(Label[G_return])))
		{
			disp_home();
			return;
		}
		else if(touch_area(&(Label[G_ctrl])))
		{
			record();
			
			GUI_ClrArea(&(Label[G_ctrl]));
			GUI_DispStr(&(Label[G_ctrl]),"ʶ����");
			
			res=spch_recg(VcBuf, &dis);
			if(dis!=dis_err)
			{
				recg_count++;
				GUI_ClrArea(&(Label[G_recg_res]));
				GUI_printf(&(Label[G_recg_res]),"ʶ����:%s",(s8 *)res);
				GUI_ClrArea(&(Label[G_mtch_dis]));
				GUI_printf(&(Label[G_mtch_dis]),"ƥ�����:%d",dis);
				GUI_ClrArea(&(Label[G_stus]));
				GUI_DispStr(&(Label[G_stus]),"������Ч");
				GUI_ClrArea(&(Label[G_count]));
				GUI_printf(&(Label[G_count]),"��ʶ��%d��",recg_count);
			}
			else
			{
				GUI_HideArea(&(Label[G_recg_res]));
				GUI_HideArea(&(Label[G_mtch_dis]));
				
				GUI_ClrArea(&(Label[G_stus]));
				GUI_DispStr(&(Label[G_stus]),"������Ч");
			}
			GUI_ClrArea(&(Label[G_ctrl]));
			GUI_DispStr(&(Label[G_ctrl]),"��ʼ");
		}
		
	}
}

int main(void)
{
	BSP_Init();
	DBPLN("SYS Init OK!");
	DBPLN("CPU Speed:%ld MHz", BSP_CPU_ClkFreq() / 1000000L);
	
	disp_home();
	
	DBPLN("disp_home");
	while(1)
	{
		wait_touch();
		if(touch_area(&(Label[G_prc])))
		{
			prc();
		}
		else if(touch_area(&(Label[G_recg])))
		{
			recg();
		}
	}
}
