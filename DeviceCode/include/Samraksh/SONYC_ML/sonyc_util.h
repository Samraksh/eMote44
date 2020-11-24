int sonyc_init_filters(void);
int sonyc_comp_filter_go(const float *src, float *dest, unsigned blocksize);
int sonyc_iir_filter_go(const float *src, float *dest, unsigned blocksize);
void sonyc_iir_filter_reset(void);
void sonyc_fir_filter_reset(void);
int sonyc_fir_tap_change(unsigned tap, float val);
