#ifndef INC_ILITEK_MAIN_H_
#define INC_ILITEK_MAIN_H_
extern int ChangeToAPMode();
extern int ChangeToBootloader();

int PanelInfor();
uint32_t RawDataInfor(void);
extern uint32_t get_file_size(char *filename);
int DearlWithFunctions(int argc, char *argv[]);
int viGetPanelInfor();
int viExitTestMode();
int viEnterTestMode();
int viSwitchMode(int mode);
int32_t GetFlashData_V6(uint32_t start, uint32_t len, char *path);
int viRemote(char *argv[]);
int viScript(char *argv[]);
int viConsoleData(char *argv[]);
#endif
