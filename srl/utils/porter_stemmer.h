#ifndef PORTER_STEMMER_H
#define PORTER_STEMMER_H

#ifdef __cplusplus
extern "C"{
#endif

struct stemmer;

extern struct stemmer * create_stemmer(void);
extern void free_stemmer(struct stemmer * z); 

extern int stem(struct stemmer * z, char * b, int k); 
#ifdef __cplusplus
}
#endif

#endif
