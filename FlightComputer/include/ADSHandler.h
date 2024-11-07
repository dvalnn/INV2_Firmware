#ifndef _ADS_HANDLER_H_
#define _ADS_HANDLER_H_

void ADS_handler_slow();
void ADS_handler_fast();
void ADS_handler_all_fast();
void ADS_handler_all_slow();
void ADS_reader(void);
bool ADS_event();

#endif