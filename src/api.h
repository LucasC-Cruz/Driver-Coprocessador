
// mapeamento
extern void* mapear();
extern void fechar(void* hps_virtual);

//flags
extern bool get_flag_busy(void* hps_virtual);
extern bool get_flag_done(void* hps_virtual);
extern bool get_flag_error(void* hps_virtual);
extern int  get_resultado(void* hps_virtual);

// ins basicas
extern void instrucao(void* hps_virtual, int inst);
extern void iniciar(void* hps_virtual);
extern void status(void* hps_virtual);

//sinais de controle
extern void enable(void* hps_virtual);
extern void reset(void* hps_virtual);
extern void clear_operation(void* hps_virtual);

extern void store_image(void* hps_virtual);
extern void store_bias(void* hps_virtual);
extern void store_beta(void* hps_virtual);
extern void store_pesos(void* hps_virtual);

extern void str_img(void* hps_virtual, int endereco, int pixel);
extern void str_bias(void* hps_virtual, int endereco, int valor);
extern void str_beta(void* hps_virtual, int endereco, int valor);
extern void str_wadress(void* hps_virtual, int endereco);
extern void str_weight(void* hps_virtual, int endereco, int valor);

extern void NO_OP(void* hps_virtual);

extern long long int confirmar(void* hps_virtual);


extern int total_inst();
