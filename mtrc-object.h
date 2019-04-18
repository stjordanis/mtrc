#pragma once


#include <stdio.h>

typedef struct mtrc_object_s
{
	/*@owned@*/ char *src_filename;     /**< Source filename */
	/*@owned@*/ char *obj_filename;     /**< Object filename */

	unsigned char* buffer;				/**< Машинный код */
	int lenght;							/**< Длина машинного кода */

	int reloc_offset;					/**< Адрес точки релокации */

	int entry_offset;					/**< Адрес точки входа _get_rule */

	int begin_state;					/**< Номер начального состояния */

} mtrc_object;

int write_object(mtrc_object& obj, FILE* fout);