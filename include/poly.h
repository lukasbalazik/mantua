#ifndef POLY_H
#define POLY_H

int start_morphing(char *);
int rewrite_text_section(int, unsigned long int, unsigned long int, char *);

char *load_text_section(int, unsigned long int, unsigned long int);
char *morph_text_section(unsigned long int *, char *);

#endif
