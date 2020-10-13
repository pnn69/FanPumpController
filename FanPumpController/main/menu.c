/*
 * menu.c
 *
 *  Created on: 24 Jun 2020
 *      Author: Peter
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "global.h"
#include "display.h"
#include "esp_log.h"
#include "struckt.h"
#include "ntc.h"
#include "timer.h"

#define TAG "menu"

#define menuhome 0
#define menuLDR 1
#define menuRH 2
#define menuFan 3
#define menuClean 4
#define menuMode 5
#define menuPID 6
#define menuInfo 7
#define menuPump 8
#define menuRHset 9
#define menuRHday 10
#define menuRHnight 11
#define menuExit 12
#define menuTout 13

static int menuPos[3]={0,0,0,};
static char tbuf[80];
extern float hum_FG6485;

//(dim2<0) ? 1:0
void OLED_homeScreen() {
	//ssd1306_display_clear();
	if(mode == modeFanAuxBox){sprintf(tbuf,"FanAuxBox   \nFan:    %03d%%\nRH set: %2.0f%%  \nRH act:%2.0f%%    ",dim1/10,Kabel400.RH_set,hum_FG6485);}
	else if(mode == modeFanAuxBoxRetro){sprintf(tbuf,"FanAuxBoxRetro\nFan: %03d%%\nOut2 %s    \nOut3 %s          ",dim1/10,(dim2>500) ? "On":"Off",(dim3>500) ? "On":"Off");}
	else if(mode == modeFanPumpController){sprintf(tbuf,"FanPumpControl\nTin:  %2.0f    \nTout: %2.0f    \nFan:%03d%% Pump:%d ",NTC[Tin],NTC[Tout],(dim1/10),(dim2<10) ? 0:1);}
	else if(mode == modeFanPumpBoxRetro){sprintf(tbuf,"FanPumpBoxRetro");}
	else sprintf(tbuf,"No setup\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	ssd1306_display_text(tbuf);
    menuPos[0] = 0;
    menuPos[1] = 0;
    menuPos[2] = 0;
}

void OLED_infoScreen() {
	ssd1306_display_clear();
	if(mode == modeFanAuxBox) sprintf(tbuf,"FanAuxBox\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	else if(mode== modeFanAuxBoxRetro)sprintf(tbuf,"FanAuxBoxRetro\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	else if(mode == modeFanPumpController)sprintf(tbuf,"PumpControl\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	else if(mode == modeFanPumpBoxRetro)sprintf(tbuf,"FanPumpBoxRetro\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	else sprintf(tbuf,"No setup\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
	ssd1306_display_text(tbuf);
    menuPos[0] = 0;
    menuPos[1] = 0;
    menuPos[2] = 0;
}

int setTOUT(sys *name,int inp){
    if(menuPos[1] == 0 && menuPos[2] == 0){
    	ssd1306_display_text("Tout        \nSETUP        ");
    	if(inp == UP){
            menuPos[2]++;
            inp=0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 0;
            inp=0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 1){
    	ssd1306_display_text("Tout        \nEXIT         ");
        if(inp == UP){
        	ssd1306_display_text("Tout        \nSETUP        ");
        	menuPos[1] = 0;
        	menuPos[2] = 0;
            inp = 0;
        }
    	if(inp == ENTER){
    		ssd1306_display_text("Tout SETUP      \n            ");
    		menuPos[0] = 0;
    		menuPos[1] = menuTout;
	        menuPos[2] = 0;
            inp = 0;
            return 1;
        }
    }
	if(menuPos[1] == 1 && menuPos[2] == 0){
		if(inp == UP && name->tout < 50){
			name->tout++;
		}
		if(inp == DOWN && name->tout > 20){
			name->tout--;
		}
		sprintf(tbuf,"Tout   \n%02.0fC      ",name->tout);
		//sprintf(tbuf,"P    \n%2f%%      ",name->P);
		ssd1306_display_text(tbuf);
		if(inp == ENTER){
			ssd1306_display_text("Tout          \nSETUP                 ");
			menuPos[1] = 0;
			menuPos[2] = 0;
			vTaskDelay(pdMS_TO_TICKS(250));
			return 0;
		}
	}
	return 0;
}
int setPID(sys *name,int inp){
    if(menuPos[1] == 0 && menuPos[2] == 0){
    	ssd1306_display_text("P           \nSETUP        ");
    	if(inp == UP){
            menuPos[2]++;
            inp=0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 0;
            inp=0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 1){
        ssd1306_display_text("I          \nSETUP        ");
        if(inp == UP){
            menuPos[2]++;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 1;
            inp = 0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 2){
        ssd1306_display_text("D          \nSETUP        ");
        if(inp == UP){
            menuPos[2]++;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 2;
            inp = 0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 3){
    	ssd1306_display_text("PID         \nEXIT         ");
        if(inp == UP){
        	ssd1306_display_text("P           \nSETUP        ");
        	menuPos[1] = 0;
        	menuPos[2] = 0;
            inp = 0;
        }
    	if(inp == ENTER){
    		ssd1306_display_text("PID SETUP       \n            ");
    		menuPos[0] = 0;
    		menuPos[1] = menuPID;
	        menuPos[2] = 0;
            inp = 0;
            return 1;
        }
    }
	if(menuPos[1] == 1 && menuPos[2] == 0){
		//ESP_LOGI(TAG,"Menu P");
		if(inp == UP && name->P < 100){
			name->P++;
		}
		if(inp == DOWN && name->P > 1){
			name->P--;
		}
		sprintf(tbuf,"P    \n%02.0f      ",name->P);
		ssd1306_display_text(tbuf);
		if(inp == ENTER){
			ssd1306_display_text("P           \nSETUP                 ");
			menuPos[1] = 0;
			menuPos[2] = 0;
			vTaskDelay(pdMS_TO_TICKS(250));
			return 0;
		}
	}
	if(menuPos[1] == 1 && menuPos[2] == 1){
		if(inp == UP && name->I < 100){
			name->I++;
		}
		if(inp == DOWN && name->I > 0){
			name->I--;
		}
		sprintf(tbuf,"I    \n%02.0f      ",name->I);
		ssd1306_display_text(tbuf);
		if(inp == ENTER){
			ssd1306_display_text("I           \nSETUP                 ");
			menuPos[1] = 0;
			menuPos[2] = 1;
			vTaskDelay(pdMS_TO_TICKS(250));
			return 0;
		}
	}
	if(menuPos[1] == 1 && menuPos[2] == 2){
		if(inp == UP && name->D < 100){
			name->D++;
		}
		if(inp == DOWN && name->D > 0){
			name->D--;
		}
		sprintf(tbuf,"D    \n%02.0f      ",name->D);
		ssd1306_display_text(tbuf);
		if(inp == ENTER){
			ssd1306_display_text("D           \nSETUP                 ");
			menuPos[1] = 0;
			menuPos[2] = 2;
			vTaskDelay(pdMS_TO_TICKS(250));
			return 0;
		}
	}
    return 0;
}

static int t=0;
void setMODE(int inp){
	if(inp == 0){
		t=mode;
	}
	if(inp == UP){
		t++;
		if(t == modeFanPumpBoxRetro){
			t++;
		}
		if(t > modeFanPumpBoxRetro){
			t = modeFanAuxBox;
		}
	}
	switch (t){
	case modeFanAuxBox:
		ssd1306_display_text("Store push DOWN\nFanAuxBox         ");
		break;
	case modeFanAuxBoxRetro:
		ssd1306_display_text("Store push DOWN\nFanAuxBoxRetro    ");
		break;
	case modeFanPumpController:
		ssd1306_display_text("Store push DOWN\nFanPumpControl    ");
		break;
	case modeFanPumpBoxRetro:
		ssd1306_display_text("Store push DOWN\nPumpBoxRetro      ");
		break;
	}
	if(inp == DOWN){
    	if( read_nvmint32("MODE") != t){
    		ESP_LOGI(TAG,"MODE set to %d",t);
    		write_nvmint32("MODE",t);
    		ssd1306_display_text("Store new data    \n                    ");
    		vTaskDelay(pdMS_TO_TICKS(250));
    		ssd1306_display_text("\n*");
    		vTaskDelay(pdMS_TO_TICKS(250));
    		ssd1306_display_text("\n**");
    		vTaskDelay(pdMS_TO_TICKS(250));
    		ssd1306_display_text("\n***");
    		vTaskDelay(pdMS_TO_TICKS(250));
    		ssd1306_display_text("\n****");
    		vTaskDelay(pdMS_TO_TICKS(250));
    		ssd1306_display_text("\n*****\nRebooting...");
    		vTaskDelay(pdMS_TO_TICKS(1000));
    		esp_restart();
    	}
	}
	if(inp == ENTER ){
    	ssd1306_display_text("MODE SETUP      \n                  ");
    	menuPos[0] = 0;
        menuPos[1] = 5;
	}
}

void setCLEAN(int inp){
	if(inp == 0){
		menuPos[1] = 0;
		menuPos[2] = 0;
	}
	if(menuPos[1] == 0 && menuPos[2] == 0){
		ssd1306_display_text("CLEAN AUTO/MAN    \nSETUP         ");
		if(inp == ENTER){
			menuPos[1] = 1;
			menuPos[2] = 0;
			inp = 0;
		}
		if(inp == UP){
			menuPos[2] = 1;
			inp = 0;
		}
	}
	if(menuPos[1] == 0 && menuPos[2] == 1){
		ssd1306_display_text("CLEAN PERIOD   \nSETUP           ");
		if(inp == ENTER){
			menuPos[1] = 1;
			menuPos[2] = 1;
			inp = 0;
		}
		if(inp == UP){
			menuPos[2] = 2;
			inp = 0;
		}
	}
	if(menuPos[1] == 0 && menuPos[2] == 2){
		ssd1306_display_text("CLEAN PERIOD   \nMANUAL SETUP     ");
		if(inp == ENTER){
			menuPos[1] = 1;
			menuPos[2] = 2;
			inp = 0;
		}
		if(inp == UP){
			menuPos[2] = 3;
			inp = 0;
		}
	}
	if(menuPos[1] == 0 && menuPos[2] == 3){
		ssd1306_display_text("CLEAN SETUP   \nEXIT           \n                    \n             ");
		if(inp == ENTER){
			ssd1306_display_text("CLEAN SETUP                                 ");
			menuPos[0] = 0;
			menuPos[1] = 4;
			menuPos[2] = 0;
			store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
			vTaskDelay(pdMS_TO_TICKS(250));
			return;
		}
		if(inp == UP){
			ssd1306_display_text("CLEAN AUTO/MAN   \nSETUP                     ");
			menuPos[1] = 0;
			menuPos[2] = 0;
			inp = 0;
		}
	}
	if(menuPos[1] == 1 && menuPos[2] == 0){
		if(inp == ENTER){
			ssd1306_display_text("CLEAN AUTO/MAN   \nSETUP                    ");
			menuPos[1] = 0;
			menuPos[2] = 0;
			vTaskDelay(pdMS_TO_TICKS(250));
			return;
		}
		if(inp == UP){Kabel400.cleanManual =! Kabel400.cleanManual;}
		if(inp == DOWN){Kabel400.cleanManual =! Kabel400.cleanManual;}
		if(Kabel400.cleanManual){sprintf(tbuf,"CLEAN:AUTO                       ");}
		else{sprintf(tbuf,"CLEAN:MANUAL                         ");}
		ssd1306_display_text(tbuf);
	}
	if(menuPos[1] == 1 && menuPos[2] == 1){
		if(inp == ENTER){
			ssd1306_display_text("CLEAN PERIOD     \nSETUP                      ");
			menuPos[1] = 0;
			menuPos[2] = 1;
			vTaskDelay(pdMS_TO_TICKS(250));
			return;
		}
		if(inp == UP){
			if(Kabel400.cleantimeset == 24)
				Kabel400.cleantimeset = 3;
			else if(Kabel400.cleantimeset == 12)
				Kabel400.cleantimeset = 24;
			else if(Kabel400.cleantimeset == 6)
				Kabel400.cleantimeset = 12;
			else if(Kabel400.cleantimeset == 6)
				Kabel400.cleantimeset = 12;
			else if(Kabel400.cleantimeset == 3)
				Kabel400.cleantimeset = 6;
			else Kabel400.cleantimeset = 6;
		}
		sprintf(tbuf,"CLEAN PERIOD    \n%d-Hr   ",Kabel400.cleantimeset);
		ssd1306_display_text(tbuf);
	}
	if(menuPos[1] == 1 && menuPos[2] == 2){
		if(inp == ENTER){
			ssd1306_display_text("CLEAN AUTO/MAN  \nSETUP         ");
			menuPos[1] = 0;
			menuPos[2] = 2;
			vTaskDelay(pdMS_TO_TICKS(250));
			return;
		}
		if(inp == DOWN){
			if(Kabel400.cleantimeset > 1)
			Kabel400.cleantimeset--;
		}
		if(inp == UP){
			Kabel400.cleantimeset++;
			if(Kabel400.cleantimeset > 72)
				Kabel400.cleantimeset =72;
		}
		sprintf(tbuf,"CLEAN PERIOD       \n%d-Hr MANUAL    ",Kabel400.cleantimeset);
		ssd1306_display_text(tbuf);
	}
	vTaskDelay(pdMS_TO_TICKS(250));
	return;
}


//call with pointer to mem
//at return 1 store the data to the struct in nvm
int setFAN(sys *name,int inp){
	if(inp == 0){
        menuPos[1] = 0;
        menuPos[2] = 0;
    }
    if(menuPos[1] == 0 && menuPos[2] == 0){
    	ssd1306_display_text("FAN MAX    \nSETUP       ");
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 1;
            inp = 0;
        }
        if(inp == ENTER){
        	menuPos[1] = 1;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    //Level1
    if(menuPos[1] == 0 && menuPos[2] == 1){
    	ssd1306_display_text("FAN MIN      \nSETUP     ");
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 2;
            inp = 0;
        }
        if(inp == ENTER){
        	menuPos[1] = 1;
        	menuPos[2] = 1;
            inp = 0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 2){
    	ssd1306_display_text("FAN SELECT   \nAUTO/MANUAL         ");
        if(inp ==UP){
            menuPos[1] = 0;
            menuPos[2] = 3;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
        	menuPos[2] = 2;
            inp = 0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 3){
    	ssd1306_display_text("FAN MANUAL   \nSETUP           ");
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 4;
            inp = 0;
        }
        if(inp == ENTER){
        	menuPos[1] = 1;
        	menuPos[2] = 3;
            inp = 0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 4){
    	ssd1306_display_text("FAN SETUP   \nEXIT         \n                ");
        if(inp == UP){
        	menuPos[1] = 0;
        	menuPos[2] = 0;
            ssd1306_display_text("FAN MAX        \nSETUP               ");
        }
        if(inp == ENTER){
        	ssd1306_display_text("FAN SETUP         \n           ");
            menuPos[0] = 0;
            menuPos[1] = 3;
            menuPos[2] = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
            return 1;
        }
    }
    if(menuPos[1] == 1 && menuPos[2] == 0){
        if(inp == UP && name->fanspeedmaximal < 100){
        	name->fanspeedmaximal++;
        }
        if(inp == DOWN && name->fanspeedmaximal > 30){
        	name->fanspeedmaximal--;
        }
        sprintf(tbuf,"FANSPEED MAX    \n%d%%      ",name->fanspeedmaximal);
        ssd1306_display_text(tbuf);
        if(inp == ENTER){
        	ssd1306_display_text("FAN MAX     \nSETUP                 ");
            menuPos[1] = 0;
            menuPos[2] = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
            return 0;
        }
    }
    if(menuPos[1] == 1 && menuPos[2] == 1){
        if(inp == UP && name->fanspeedminimal < 40){
        	name->fanspeedminimal++;
        }
        if(inp == DOWN && name->fanspeedminimal != 0){
        	name->fanspeedminimal--;
        }

        sprintf(tbuf,"FANSPEED MIN  \n%d%%        ",name->fanspeedminimal);
        ssd1306_display_text(tbuf);
        if(inp == ENTER){
        	ssd1306_display_text("FAN MIN      \nSETUP           ");
            menuPos[1] = 0;
            menuPos[2] = 1;
            vTaskDelay(pdMS_TO_TICKS(250));
            return 0;
        }
    }
    if(menuPos[1] == 1 && menuPos[2] == 2){
        if(inp == UP){name->fanManual = !name->fanManual;}
        if(inp == DOWN){name->fanManual = !name->fanManual;}
        if(name->fanManual){sprintf(tbuf,"FAN          \nFAN:AUTO    ");}
        else{sprintf(tbuf,"FAN          \nFAN:MANUAL  ");}
        ssd1306_display_text(tbuf);
        if(inp == ENTER){
        	ssd1306_display_text("FAN SELECT        \nAUTO/MANUAL    ");
            menuPos[1] = 0;
            menuPos[2] = 2;
            vTaskDelay(pdMS_TO_TICKS(250));
            return 0;
        }
    }
    if(menuPos[1] == 1 && menuPos[2] == 3){
        if(inp == ENTER){
        	ssd1306_display_text("FAN MANUAL      \nSETUP     ");
            menuPos[1] = 0;
            menuPos[2] = 3;
            vTaskDelay(pdMS_TO_TICKS(250));
            return 0;
        }
        if(inp == DOWN){
            if(name->fanspeedmanual > name->fanspeedminimal){
            	name->fanspeedmanual--;
            }
        }
        if(inp == UP){
        	name->fanspeedmanual++;
            if(name->fanspeedmanual > name->fanspeedmaximal ){
            	name->fanspeedmanual = name->fanspeedmaximal;
            }
        }
        sprintf(tbuf,"FANSPEED MAN\n%d%%    ",name->fanspeedmanual);
        ssd1306_display_text(tbuf);
    }
    vTaskDelay(pdMS_TO_TICKS(250));
    return 0;
}

int setPUMP(sys *name,int inp){
	if(inp == 0){
        menuPos[1] = 0;
        menuPos[2] = 0;
    }
    if(menuPos[1] == 0 && menuPos[2] == 0){
    	ssd1306_display_text("PUMP MAX    \nSETUP       ");
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 1;
            inp = 0;
        }
        if(inp == ENTER){
        	menuPos[1] = 1;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    //Level1
    if(menuPos[1] == 0 && menuPos[2] == 1){
    	ssd1306_display_text("PUMP MIN      \nSETUP     ");
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 2;
            inp = 0;
        }
        if(inp == ENTER){
        	menuPos[1] = 1;
        	menuPos[2] = 1;
            inp = 0;
        }
    }
    if(menuPos[1] == 0 && menuPos[2] == 2){
    	ssd1306_display_text("PUMP SETUP   \nEXIT         \n                ");
        if(inp == UP){
        	menuPos[1] = 0;
        	menuPos[2] = 0;
            ssd1306_display_text("PUMP MAX        \nSETUP               ");
        }
        if(inp == ENTER){
        	ssd1306_display_text("PUMP SETUP         \n           ");
            menuPos[0] = 0;
            menuPos[1] = menuPump;
            menuPos[2] = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
            return 1;
        }
    }
    if(menuPos[1] == 1 && menuPos[2] == 0){
        if(inp == UP && name->pumpspeedmaximal < 60){
                name->pumpspeedmaximal++;
        }
        if(inp == DOWN && name->pumpspeedmaximal > 20 &&  name->pumpspeedmaximal > name->pumpspeedminimal){
                name->pumpspeedmaximal--;
        }
        sprintf(tbuf,"PUMPSPEED MAX    \n%d%%      ",name->pumpspeedmaximal);
        ssd1306_display_text(tbuf);
        if(inp == ENTER){
        	ssd1306_display_text("PUMP MAX     \nSETUP                 ");
            menuPos[1] = 0;
            menuPos[2] = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
    if(menuPos[1] == 1 && menuPos[2] == 1){
        if(inp == UP && name->pumpspeedminimal < 40 && name->pumpspeedminimal < name->pumpspeedmaximal ){
                name->pumpspeedminimal++;
        }
        if(inp == DOWN && name->pumpspeedminimal != 0){
                name->pumpspeedminimal--;
        }

        sprintf(tbuf,"PUMPSPEED MIN  \n%d%%        ",name->pumpspeedminimal);
        ssd1306_display_text(tbuf);
        if(inp == ENTER){
        	ssd1306_display_text("PUMP MIN      \nSETUP           ");
            menuPos[1] = 0;
            menuPos[2] = 1;
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
    return 0;
}

void setRH(int inp){
	if(inp == 0){
    	ssd1306_display_text("RH SETPOINT \nSETUP        ");
        menuPos[1] = 0;
        menuPos[2] = 0;
        vTaskDelay(pdMS_TO_TICKS(250));
        return;
    }
    if(menuPos[1] == 0 && menuPos[2] == 0){
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 1;
            inp=0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 0;
            inp=0;
        }
        ssd1306_display_text("RH SETPOINT \nSETUP        ");
    }

    if(menuPos[1] == 0 && menuPos[2] == 1){
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 2;
            inp=0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 1;
            inp=0;
        }
        ssd1306_display_text("RH DAY       \nSETUP      ");
    }

    if(menuPos[1] == 0 && menuPos[2] == 2){
        if(inp == UP){
            menuPos[1] = 0;
            menuPos[2] = 3;
            inp=0;
        }
        if(inp == ENTER){
            menuPos[1] = 1;
            menuPos[2] = 2;
            inp=0;
        }
        ssd1306_display_text("RH NIGHT     \nSETUP      ");
    }

    if(menuPos[1] == 0 && menuPos[2] == 3){
        if(inp == UP){
        	ssd1306_display_text("RH SETPOINT   \nSETUP       ");
            menuPos[1] = 0;
            menuPos[2] = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        if(inp == ENTER){
        	ssd1306_display_text("RH SETUP  \n             ");
            menuPos[0] = 0;
            menuPos[1] = 2;
            menuPos[2] = 0;
            store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        ssd1306_display_text("RH SETUP  \nEXIT          \n          ");
    }


//level 2
    if(menuPos[1] == 1 && menuPos[2] == 0){
        if(inp == UP && Kabel400.RH_set < Kabel400.RHmaximal){
                Kabel400.RH_set += 1;
        }
        if(inp == DOWN && Kabel400.RH_set > Kabel400.RHmimimal){
                Kabel400.RH_set -= 1;
        }
        if(inp == ENTER){
        	ssd1306_display_text("RH SETPIONT \nSETUP         ");
            menuPos[1] = 0;
            menuPos[2] = 0;
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        sprintf(tbuf,"RH SETPOINT\n%02.0f%%         ",Kabel400.RH_set);
        ssd1306_display_text(tbuf);
    }

    if(menuPos[1] == 1 && menuPos[2] == 1){
        if(inp == UP && Kabel400.RH_day < Kabel400.RHmaximal){
                Kabel400.RH_day += 1;
                ESP_LOGI(TAG,"%03d",Kabel400.RH_day);
        }
        if(inp == DOWN && Kabel400.RH_day > Kabel400.RHmimimal){
                Kabel400.RH_day -= 1;
                ESP_LOGI(TAG,"%03d",Kabel400.RH_day);
        }
        if(inp == ENTER){
        	ssd1306_display_text("RH DAY      \nSETUP         ");
            menuPos[1] = 0;
            menuPos[2] = 1;
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        sprintf(tbuf,"RH DAY\n%d%%         ",Kabel400.RH_day);
        ssd1306_display_text(tbuf);
    }

    if(menuPos[1] == 1 && menuPos[2] == 2){
        if(inp == UP && Kabel400.RH_night < Kabel400.RHmaximal){
                Kabel400.RH_night += 1;
        }
        if(inp == DOWN && Kabel400.RH_night > Kabel400.RHmimimal){
                Kabel400.RH_night -= 1;
        }
        if(inp == ENTER){
        	ssd1306_display_text("RH NIGHT   \nSETUP           ");
            menuPos[1] = 0;
            menuPos[2] = 2;
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        sprintf(tbuf,"RH NIGHT     \n%d%%     ",Kabel400.RH_night);
        ssd1306_display_text(tbuf);
    }
}

void setLDR(int inp){
	if(inp == 0){
		vTaskDelay(pdMS_TO_TICKS(250));
	}
	if(inp == UP || inp == DOWN){
        Kabel400.ldr = !Kabel400.ldr;
    }
    if(inp == ENTER  ){
    	ssd1306_display_clear();
    	ssd1306_display_text("LDR SETUP");
        menuPos[0] = 0;
        menuPos[1] = 1;
        store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
        vTaskDelay(pdMS_TO_TICKS(250));
        return;
    }
    if(Kabel400.ldr == true){ssd1306_display_text("LDR ON      \nON/OFF    ");}
    else{ssd1306_display_text("LDR OFF      \nON/OFF    ");}
    vTaskDelay(pdMS_TO_TICKS(250));
    return;
}

void setINFO(int inp){
	if(inp == UP || inp == ENTER){
		ssd1306_display_clear();
		OLED_homeScreen();
	}
	if(inp == DOWN){
		ssd1306_display_clear();
		if(mode == modeFanAuxBox) sprintf(tbuf,"FanAuxBox\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
		else if(mode== modeFanAuxBoxRetro)sprintf(tbuf,"FanAuxBoxRetro\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
		else if(mode == modeFanPumpController)sprintf(tbuf,"PumpControl\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
		else if(mode == modeFanPumpBoxRetro)sprintf(tbuf,"FanPumpBoxRetro\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
		else sprintf(tbuf,"No setup\nHW:%s\nSW:%s\nMEM:%d",HW_VERSION,SW_VERSION,NVM_VERSION);
		ssd1306_display_text(tbuf);
	}
}

//**************************************************************************************************
void LCD_menu_1(int inp){
	if(inp == 0 && menuPos[0] == 0 && menuPos[1] == 0 && menuPos[2] == 0 ){
		OLED_homeScreen();
		return;
	}
	if(inp == 0){
		return;
	}
    if(menuPos[0] == 0 && menuPos[1] == 0){
        if(inp == ENTER){
            OLED_homeScreen();
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        if(inp == UP){
        	ssd1306_display_clear();
            menuPos[1] = menuLDR;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuLDR){
    	ssd1306_display_text("LDR SETUP         ");
        if(inp == UP){ //LDR
            menuPos[1] = menuRH;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuLDR;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuRH){
    	ssd1306_display_text("RH SETUP            ");
        if(inp == UP){ //RH
            menuPos[1]++;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuRH;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuFan){
    	ssd1306_display_text("FAN SETUP         ");
        if(inp == UP){ //FAN
            menuPos[1] = menuClean;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuFan;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuClean){
    	ssd1306_display_text("CLEAN SETUP        ");
        if(inp == UP){ //CLEAN
            menuPos[1] = menuMode;
            inp = 0;
        }
        if(inp == ENTER){
        	ssd1306_display_text("LDR SETUP       ");
            menuPos[0] = menuClean;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }

    if(menuPos[0] == 0 && menuPos[1] == menuMode){
    	ssd1306_display_text("MODE SETUP          ");
        if(inp == UP){
            menuPos[1] = menuPID;
            inp = 0;
        }
        if(inp == ENTER){
        	ssd1306_display_clear();
            menuPos[0] = menuMode;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }

    if(menuPos[0] == 0 && menuPos[1] == menuPID){
    	ssd1306_display_text("PID SETUP         ");
        if(inp == UP){
            menuPos[1] = menuInfo;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuPID;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }

    if(menuPos[0] == 0 && menuPos[1] == menuInfo){
    	ssd1306_display_text("SYS INFO          ");
        if(inp == UP){
            menuPos[1] = menuExit;
            inp = 0;
        }
        if(inp == ENTER){
        	OLED_infoScreen();
            menuPos[0] = menuInfo;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = DOWN;
        }
    }

    if(menuPos[0] == 0 && menuPos[1] == menuExit){
    	ssd1306_display_text("EXIT           ");
        if(inp == UP){
        	ssd1306_display_text("LDR SETUP         ");
            menuPos[1] = menuLDR;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 0;
            OLED_homeScreen();
        }
    }
    //Show menu's
    if(menuPos[0] == menuLDR ){
    	setLDR(inp);
    }
    if(menuPos[0] == menuRH ){ //main 2,0,0
    	setRH(inp);
    }
    if(menuPos[0] == menuFan ){
    	if( setFAN(&Kabel400,inp)){
    		store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
    	}
    }
    if(menuPos[0] == menuClean ){
    	setCLEAN(inp);
    }
    if(menuPos[0] == menuMode ){
    	setMODE(inp);
    }
    if(menuPos[0] == menuPID ){
    	if(setPID(&Kabel400,inp)){
    		store_struckt_name("Kabel400",&Kabel400,sizeof(Kabel400));
    	}
    }
    if(menuPos[0] == menuInfo ){
    	setINFO(inp);
    }
    inp=0;
    vTaskDelay(pdMS_TO_TICKS(250));
}

//**********************************************************************
void LCD_menu_2(int inp){
	if(inp == 0 && menuPos[0] == 0 && menuPos[1] == 0 && menuPos[2] == 0 ){
		OLED_homeScreen();
		return;
	}
	if(inp == 0){
		return;
	}
    if(menuPos[0] == 0 && menuPos[1] == 0){
        if(inp == ENTER){
            OLED_homeScreen();
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        if(inp == UP){
        	ssd1306_display_clear();
            menuPos[1] = menuMode;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuMode){
    	ssd1306_display_text("MODE SETUP          ");
        if(inp == UP){
            menuPos[1] = menuExit;
            inp = 0;
        }
        if(inp == ENTER){
        	ssd1306_display_clear();
            menuPos[0] = menuMode;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuExit){
    	ssd1306_display_text("EXIT           ");
        if(inp == UP){
        	ssd1306_display_text("MODE SETUP          ");
            menuPos[1] = menuMode;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 0;
            OLED_homeScreen();
        }
    }
    if(menuPos[0] == menuMode ){
    	setMODE(inp);
    }
	vTaskDelay(pdMS_TO_TICKS(250));
}

//**********************************************************************
void LCD_menu_3(int inp){
	if(inp == 0 && menuPos[0] == 0 && menuPos[1] == 0 && menuPos[2] == 0 ){
		OLED_homeScreen();
		return;
	}
	if(inp == 0){
		return;
	}
    if(menuPos[0] == 0 && menuPos[1] == 0){
        if(inp == ENTER){
            OLED_homeScreen();
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        if(inp == UP){
        	ssd1306_display_clear();
            menuPos[1] = menuTout;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuTout){
    	ssd1306_display_text("Tout SETUP        \n               ");
        if(inp == UP){
            menuPos[1] = menuFan;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuTout;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuFan){
    	ssd1306_display_text("FAN SETUP        \n               ");
        if(inp == UP){ //FAN
            menuPos[1] = menuPump;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuFan;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuPump){
    	ssd1306_display_text("Pump SETUP       \n              ");
        if(inp == UP){
            menuPos[1] = menuPID;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuPump;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuPID){
    	ssd1306_display_text("PID SETUP          \n            ");
        if(inp == UP){
            menuPos[1]= menuMode;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[0] = menuPID;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuMode){
    	ssd1306_display_text("MODE SETUP         \n             ");
        if(inp == UP){
            menuPos[1] = menuInfo;
            inp = 0;
        }
        if(inp == ENTER){
        	ssd1306_display_clear();
            menuPos[0] = menuMode;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuInfo){
    	ssd1306_display_text("SYS INFO          \n               ");
        if(inp == UP){
            menuPos[1] = menuExit;
            inp = 0;
        }
        if(inp == ENTER){
        	OLED_infoScreen();
            menuPos[0] = menuInfo;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = DOWN;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuExit){
    	ssd1306_display_text("EXIT          \n                ");
        if(inp == UP){
        	ssd1306_display_text("Tout SETUP        \n               ");
            menuPos[1] = menuTout;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 0;
            OLED_homeScreen();
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
    }

    //Show menu's here
    if(menuPos[0] == menuLDR ){
    	setLDR(inp);
    }
    if(menuPos[0] == menuRH ){
    	setRH(inp);
    }
    if(menuPos[0] == menuTout ){
    	if( setTOUT(&Pump,inp)){
    		store_struckt_name("Pump",&Pump,sizeof(Pump));
    	}
    }
    if(menuPos[0] == menuFan ){
    	if( setFAN(&Pump,inp)){
    		store_struckt_name("Pump",&Pump,sizeof(Pump));
    	}
    }
    if(menuPos[0] == menuPump ){
    	if( setPUMP(&Pump,inp)){
    		store_struckt_name("Pump",&Pump,sizeof(Pump));
    	}
    }
    if(menuPos[0] == menuMode ){
		setMODE(inp);
    }
    if(menuPos[0] == menuPID ){
    	if( setPID(&Pump,inp)){
    		store_struckt_name("Pump",&Pump,sizeof(Pump));
    	}
    }
    if(menuPos[0] == menuInfo ){
    	setINFO(inp);
    }
    inp = 0;
	vTaskDelay(pdMS_TO_TICKS(250));
}
//**********************************************************************
void LCD_menu_4(int inp){
	if(inp == 0 && menuPos[0] == 0 && menuPos[1] == 0 && menuPos[2] == 0 ){
		OLED_homeScreen();
		return;
	}
	if(inp == 0){
		return;
	}
    if(menuPos[0] == 0 && menuPos[1] == 0){
        if(inp == ENTER){
            OLED_homeScreen();
            vTaskDelay(pdMS_TO_TICKS(250));
            return;
        }
        if(inp == UP){
        	ssd1306_display_clear();
            menuPos[1] = menuMode;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuMode){
    	ssd1306_display_text("MODE SETUP          ");
        if(inp == UP){
            menuPos[1] = menuExit;
            inp = 0;
        }
        if(inp == ENTER){
        	ssd1306_display_clear();
            menuPos[0] = menuMode;
            menuPos[1] = 0;
            menuPos[2] = 0;
            inp = 0;
        }
    }
    if(menuPos[0] == 0 && menuPos[1] == menuExit){
    	ssd1306_display_text("EXIT           ");
        if(inp == UP){
        	ssd1306_display_text("MODE SETUP          ");
            menuPos[1] = menuMode;
            inp = 0;
        }
        if(inp == ENTER){
            menuPos[1] = 0;
            OLED_homeScreen();
        }
    }
    if(menuPos[0] == menuMode ){
    	setMODE(inp);
    }
	vTaskDelay(pdMS_TO_TICKS(250));
}
