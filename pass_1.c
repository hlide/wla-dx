
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#include "defines.h"

#include "main.h"
#include "include_file.h"
#include "parse.h"
#include "pass_1.h"
#include "pass_2.h"
#include "pass_3.h"
#include "stack.h"
#include "hashmap.h"
#include "printf.h"
#include "mersenne_twister.h"


#ifdef GB
char g_licenseecodenew_c1, g_licenseecodenew_c2;
int g_gbheader_defined = 0;
int g_nintendologo_defined = 0;
int g_computechecksum_defined = 0, g_computecomplementcheck_defined = 0;
int g_romgbc = 0, g_romdmg = 0, g_romsgb = 0;
int g_cartridgetype = 0, g_cartridgetype_defined = 0;
int g_countrycode = 0, g_countrycode_defined = 0;
int g_licenseecodenew_defined = 0, g_licenseecodeold = 0, g_licenseecodeold_defined = 0;
int g_version = 0, g_version_defined = 0;
#endif

#ifdef Z80
char g_sdsctag_name_str[MAX_NAME_LENGTH + 1], g_sdsctag_notes_str[MAX_NAME_LENGTH + 1], g_sdsctag_author_str[MAX_NAME_LENGTH + 1];
int g_sdsctag_name_type, g_sdsctag_notes_type, g_sdsctag_author_type, g_sdsc_ma, g_sdsc_mi;
int g_sdsctag_name_value, g_sdsctag_notes_value, g_sdsctag_author_value;
int g_computesmschecksum_defined = 0, g_sdsctag_defined = 0, g_smstag_defined = 0;
int g_smsheader_defined = 0, g_smsversion = 0, g_smsversion_defined = 0, g_smsregioncode = 0, g_smsregioncode_defined = 0;
int g_smsproductcode_defined = 0, g_smsproductcode1 = 0, g_smsproductcode2 = 0, g_smsproductcode3 = 0, g_smsreservedspace1 = 0;
int g_smsreservedspace2 = 0, smsreservedspace_defined = 0, g_smsromsize = 0, g_smsromsize_defined = 0;
#endif

int g_org_defined = 1, g_background_defined = 0, g_background_size = 0;
int g_enumid_defined = 0, g_enumid = 0, g_enumid_adder = 1, g_enumid_export = 0;
int g_bank = 0, g_bank_defined = 1;
int g_rombanks = 0, g_rombanks_defined = 0, g_romtype = 0, g_max_address = 0;
int g_rambanks = 0, g_rambanks_defined = 0;
int g_emptyfill, g_emptyfill_defined = 0;
int g_section_status = OFF, g_section_id = 1, g_line_count_status = ON;
int d, g_source_pointer, ind, inz, g_ifdef = 0, t, g_slots_amount = 0;
int g_memorymap_defined = 0;
int g_defaultslot_defined = 0, g_defaultslot, g_current_slot = 0;
int g_banksize_defined = 0, g_banksize;
int g_rombankmap_defined = 0, *g_banks = NULL, *g_bankaddress = NULL;
int g_bankheader_status = OFF;
int g_macro_active = 0;
int g_repeat_active = 0;
int g_smc_defined = 0;
int g_asciitable_defined = 0;
int g_block_status = 0, g_block_name_id = 0;
int g_dstruct_status = OFF;
unsigned char g_asciitable[256];

#ifndef GB
extern struct stack *g_stacks_header_first, *g_stacks_header_last;
extern int g_stacks_inside, g_stacks_outside;
int g_stack_inserted;
#endif

#ifdef W65816
char g_snesid[4];
int g_snesheader_defined = 0, g_snesid_defined = 0, g_snesromsize = 0;
int g_sramsize_defined = 0, g_sramsize = 0, g_country_defined = 0, g_country = 0;
int g_cartridgetype = 0, g_cartridgetype_defined = 0, g_licenseecode_defined = 0, g_licenseecode = 0;
int g_version_defined = 0, g_version = 0, g_snesnativevector_defined = 0, g_snesemuvector_defined = 0;
int g_hirom_defined = 0, g_lorom_defined = 0, g_slowrom_defined = 0, g_fastrom_defined = 0, g_snes_mode = 0;
int g_exlorom_defined = 0, g_exhirom_defined = 0;
int g_computesneschecksum_defined = 0, g_use_wdc_standard = 0;
#endif

#if defined(GB) || defined(W65816)
char g_name[32];
int g_name_defined = 0;
#endif

char tmp[4096], g_error_message[sizeof(tmp) + MAX_NAME_LENGTH + 1 + 1024];
char *g_label_stack[256];
char cp[MAX_NAME_LENGTH + 1];

unsigned char *g_rom_banks = NULL, *g_rom_banks_usage_table = NULL;

struct export_def *g_export_first = NULL, *g_export_last = NULL;
struct optcode *g_opt_tmp;
struct definition *g_tmp_def;
struct map_t *g_defines_map = NULL;
struct macro_static *g_macros_first = NULL, *g_macros_last = NULL;
struct section_def *g_sections_first = NULL, *g_sections_last = NULL, *g_sec_tmp, *g_sec_next;
struct macro_runtime *g_macro_stack = NULL, *g_macro_runtime_current = NULL;
struct repeat_runtime *g_repeat_stack = NULL;
struct slot g_slots[256];
struct structure *g_structures_first = NULL;
struct filepointer *g_filepointers = NULL;
struct map_t *g_namespace_map = NULL;
struct append_section *g_append_sections = NULL;
struct label_sizeof *g_label_sizeofs = NULL;
struct block_name *g_block_names = NULL;
struct stringmaptable *g_stringmaptables = NULL;

extern char *g_buffer, *unfolded_buffer, g_label[MAX_NAME_LENGTH + 1], *g_include_dir, *g_full_name;
extern int g_size, g_input_number_error_msg, g_verbose_mode, g_output_format, g_open_files;
extern int g_stack_id, g_latest_stack, g_ss, g_commandline_parsing, g_newline_beginning, g_expect_calculations;
extern int g_extra_definitions, g_string_size, g_input_float_mode, g_operand_hint, g_operand_hint_type;
extern int g_include_dir_size, g_parse_floats, g_listfile_data, g_quiet, g_parsed_double_decimal_numbers;
extern int g_create_sizeof_definitions;
extern FILE *g_file_out_ptr;
extern double g_parsed_double;
extern char *g_final_name;

extern struct active_file_info *g_active_file_info_first, *g_active_file_info_last, *g_active_file_info_tmp;
extern struct file_name_info *g_file_name_info_first, *g_file_name_info_last, *g_file_name_info_tmp;
extern struct stack *g_stacks_first, *g_stacks_tmp, *g_stacks_last;
extern struct incbin_file_data *g_incbin_file_data_first, *g_ifd_tmp;

static int g_macro_stack_size = 0, g_repeat_stack_size = 0;

#if defined(MCS6502) || defined(WDC65C02) || defined(CSG65CE02) || defined(MCS6510) || defined(W65816) || defined(HUC6280) || defined(MC6800) || defined(MC6801) || defined(MC6809)
int g_xbit_size = 0, g_accu_size = 8, g_index_size = 8;
#endif

/* vars used when in an enum, ramsection, or struct. */
/* some variables named "enum_" are used in enums, ramsections, and structs. */
int in_enum = NO, in_ramsection = NO, in_struct = NO;
int enum_exp, enum_ord;
int enum_offset; /* Offset relative to enum start where we're at right now */
int last_enum_offset;
int base_enum_offset; /* start address of enum */
int enum_sizeof_pass; /* set on second pass through enum/ramsection, generating _sizeof labels */
/* temporary struct used to build up enums/ramsections (and, of course, structs)
   this gets temporarily replaced when inside a union (each union is considered a separate struct). */
struct structure *active_struct;

int union_base_offset; /* start address of current union */
int max_enum_offset; /* highest position seen within current union group */
struct structure *union_first_struct; /* first struct in current union */
struct union_stack *union_stack; /* stores variables for nested unions */

/* for .TABLE, .DATA and .ROW */
char table_format[256];
int table_defined = 0, table_size = 0, table_index = 0;

extern int opcode_n[256], opcode_p[256];
extern struct optcode opt_table[];

#define no_library_files(name)                                          \
  do {                                                                  \
    if (g_output_format == OUTPUT_LIBRARY) {                              \
      print_error("Library files don't take " name ".\n", ERROR_DIR);   \
      return FAILED;                                                    \
    }                                                                   \
  } while (0)


int strcaselesscmp(char *s1, char *s2) {

  if (s1 == NULL || s2 == NULL)
    return 0;

  while (*s1 != 0) {
    if (toupper((int)*s1) != toupper((int)*s2))
      return 1;
    s1++;
    s2++;
  }

  if (*s2 != 0)
    return 1;

  return 0;
}


static int _get_slot_number_by_its_name(char *slot_name, int *number) {

  int i;
  
  for (i = 0; i < g_slots_amount; i++) {
    if (strcmp(slot_name, g_slots[i].name) == 0) {
      *number = i;
      return SUCCEEDED;
    }
  }

  snprintf(g_error_message, sizeof(g_error_message), "Cannot find SLOT \"%s\".\n", slot_name);
  print_error(g_error_message, ERROR_DIR);

  return FAILED;  
}


static int _get_slot_number_by_a_value(int value, int *slot) {

  int i;

  if (value < 0) {
    *slot = value;
    return FAILED;
  }
  
  if (value < g_slots_amount) {
    /* value can be the direct SLOT ID, but can it be a SLOT's address as well? */
    for (i = 0; i < g_slots_amount; i++) {
      if (g_slots[i].address == value && value != i) {
        snprintf(g_error_message, sizeof(g_error_message), "There is a SLOT number %d, but there also is a SLOT (with ID %d) with starting address %d ($%x)... Using SLOT %d.\n", value, i, value, value, value);
        print_error(g_error_message, ERROR_WRN);
        break;
      }
    }
    
    *slot = value;
    return SUCCEEDED;
  }

  for (i = 0; i < g_slots_amount; i++) {
    if (g_slots[i].address == value) {
      *slot = i;
      return SUCCEEDED;
    }
  }

  *slot = -1;

  snprintf(g_error_message, sizeof(g_error_message), "Cannot find SLOT %d.\n", value);
  print_error(g_error_message, ERROR_DIR);
  
  return FAILED;
}


int macro_get(char *name, int add_namespace, struct macro_static **macro_out) {

  struct macro_static *macro;
  char fullname[MAX_NAME_LENGTH + 1];

  strcpy(fullname, name);

  /* append the namespace, if this file uses if */
  if (add_namespace == YES && g_active_file_info_last->namespace[0] != 0) {
    if (add_namespace_to_string(fullname, sizeof(fullname), "MACRO") == FAILED) {
      *macro_out = NULL;
      return FAILED;
    }
  }

  macro = g_macros_first;
  while (macro != NULL) {
    if (strcmp(macro->name, fullname) == 0)
      break;
    macro = macro->next;
  }

  *macro_out = macro;
  
  return SUCCEEDED;
}


int macro_stack_grow(void) {

  if (g_macro_active == g_macro_stack_size) {

    struct macro_runtime *macro;
    int old_size;

    /* enlarge the macro stack */
    old_size = g_macro_stack_size;
    g_macro_stack_size = (g_macro_stack_size<<1)+2;

    macro = calloc(sizeof(struct macro_runtime) * g_macro_stack_size, 1);
    if (macro == NULL) {
      print_error("Out of memory error while enlarging macro stack buffer.\n", ERROR_ERR);
      return FAILED;
    }

    if (g_macro_stack != NULL) {
      memcpy(macro, g_macro_stack, sizeof(struct macro_runtime) * old_size);
      free(g_macro_stack);
    }
    g_macro_stack = macro;
    g_macro_runtime_current = &g_macro_stack[g_macro_active - 1];
  }

  return SUCCEEDED;
}


int macro_start(struct macro_static *m, struct macro_runtime *mrt, int caller, int nargs) {

  g_macro_runtime_current = mrt;
  g_macro_active++;
  m->calls++;

  /* macro call start */
  fprintf(g_file_out_ptr, "i%s ", m->name);
  
  mrt->caller = caller;
  mrt->macro = m;
  mrt->macro_return_i = g_source_pointer;
  mrt->macro_return_line = g_active_file_info_last->line_current;
  mrt->macro_return_filename_id = g_active_file_info_last->filename_id;

  if ((g_extra_definitions == ON) && (g_active_file_info_last->filename_id != m->filename_id)) {
    redefine("WLA_FILENAME", 0.0, get_file_name(m->filename_id), DEFINITION_TYPE_STRING, (int)strlen(get_file_name(m->filename_id)));
    redefine("wla_filename", 0.0, get_file_name(m->filename_id), DEFINITION_TYPE_STRING, (int)strlen(get_file_name(m->filename_id)));
  }

  g_active_file_info_last->line_current = m->start_line;
  g_active_file_info_last->filename_id = m->filename_id;
  g_source_pointer = m->start;

  /* redefine NARGS */
  if (redefine("NARGS", (double)nargs, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;
  if (redefine("nargs", (double)nargs, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


int macro_start_dxm(struct macro_static *m, int caller, char *name, int first) {

  struct macro_runtime *mrt;
  int start;
  
  /* start running a macro... run until .ENDM */
  mrt = &g_macro_stack[g_macro_active];

  start = g_source_pointer;

  if (first == NO && mrt->string_current < mrt->string_last) {
    inz = SUCCEEDED;
    d = mrt->string[mrt->string_current++];
  }
  else {
    inz = input_number();
    if (mrt != NULL) {
      mrt->string_current = 0;
      mrt->string_last = 0;
    }
  }

  if (first == YES) {
    if (mrt != NULL)
      mrt->offset = 0;
  }
  else {
    if (caller == MACRO_CALLER_DBM)
      mrt->offset++;
    else
      mrt->offset += 2;
  }

  if (inz == INPUT_NUMBER_EOL && first == NO) {
    next_line();
    return SUCCEEDED;
  }

  if (macro_stack_grow() == FAILED)
    return FAILED;

  mrt = &g_macro_stack[g_macro_active];
  mrt->argument_data = calloc(sizeof(struct macro_argument *) << 1, 1);
  mrt->argument_data[0] = calloc(sizeof(struct macro_argument), 1);
  mrt->argument_data[1] = calloc(sizeof(struct macro_argument), 1);
  if (mrt->argument_data == NULL || mrt->argument_data[0] == NULL || mrt->argument_data[1] == NULL) {
    print_error("Out of memory error while collecting macro arguments.\n", ERROR_NONE);
    return FAILED;
  }

  mrt->argument_data[1]->type = SUCCEEDED;
  mrt->argument_data[1]->value = mrt->offset;

  /* filter all the data through that macro */
  mrt->argument_data[0]->start = start;
  mrt->argument_data[0]->type = inz;

  if (inz == FAILED)
    return FAILED;
  else if (inz == INPUT_NUMBER_EOL) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs data.\n", name);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  mrt->supplied_arguments = 2;

  if (inz == INPUT_NUMBER_ADDRESS_LABEL)
    strcpy(mrt->argument_data[0]->string, g_label);
  else if (inz == INPUT_NUMBER_STRING) {
    mrt->argument_data[0]->type = SUCCEEDED;
    mrt->argument_data[0]->value = g_label[0];
    strcpy(mrt->string, g_label);
    mrt->string_current = 1;
    mrt->string_last = (int)strlen(g_label);
    /*
      fprintf(stderr, "got string %s!\n", label);
    */
  }
  else if (inz == INPUT_NUMBER_STACK)
    mrt->argument_data[0]->value = (double)g_latest_stack;
  else if (inz == SUCCEEDED)
    mrt->argument_data[0]->value = d;
  else
    return FAILED;

  if (macro_start(m, mrt, caller, 1) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


int macro_start_incbin(struct macro_static *m, struct macro_incbin *incbin_data, int first) {

  struct macro_runtime *mrt;

  /* start running a macro... run until .ENDM */
  if (macro_stack_grow() == FAILED)
    return FAILED;

  mrt = &g_macro_stack[g_macro_active];

  if (first == YES)
    mrt->incbin_data = incbin_data;
  else
    incbin_data = mrt->incbin_data;

  if (incbin_data->left == 0)
    return SUCCEEDED;

  if (first == YES)
    mrt->offset = 0;
  else
    mrt->offset++;

  mrt->argument_data = calloc(sizeof(struct macro_argument *) << 1, 1);
  mrt->argument_data[0] = calloc(sizeof(struct macro_argument), 1);
  mrt->argument_data[1] = calloc(sizeof(struct macro_argument), 1);
  if (mrt->argument_data == NULL || mrt->argument_data[0] == NULL || mrt->argument_data[1] == NULL) {
    print_error("Out of memory error while collecting macro arguments.\n", ERROR_NONE);
    return FAILED;
  }

  /* filter all the data through that macro */
  mrt->argument_data[1]->type = SUCCEEDED;
  mrt->argument_data[1]->value = mrt->offset;
  mrt->argument_data[0]->start = g_source_pointer;
  mrt->argument_data[0]->type = SUCCEEDED;
  mrt->supplied_arguments = 2;

  if (incbin_data->swap != 0) {
    if (incbin_data->swap == 1) {
      mrt->argument_data[0]->value = incbin_data->data[incbin_data->position + 1];
      incbin_data->swap = 2;
    }
    else {
      mrt->argument_data[0]->value = incbin_data->data[incbin_data->position];
      incbin_data->position += 2;
      incbin_data->swap = 1;
    }
  }
  else
    mrt->argument_data[0]->value = incbin_data->data[incbin_data->position++];

  incbin_data->left--;

  if (macro_start(m, mrt, MACRO_CALLER_INCBIN, 1) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


int macro_insert_byte_db(char *name) {

  struct definition *d;
  
  if (hashmap_get(g_defines_map, "_out", (void*)&d) != MAP_OK)
    hashmap_get(g_defines_map, "_OUT", (void*)&d);

  if (d == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "No \"_OUT/_out\" defined, .%s takes its output from there.\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (d->type == DEFINITION_TYPE_VALUE) {
    if (d->value < -128 || d->value > 255) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s expects 8-bit data, %d is out of range!\n", name, (int)d->value);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    fprintf(g_file_out_ptr, "d%d ", (int)d->value);
    /*
      fprintf(stderr, ".DBM: VALUE: %d\n", (int)d->value);
    */
  }
  else if (d->type == DEFINITION_TYPE_STACK) {
    fprintf(g_file_out_ptr, "c%d ", (int)d->value);
    /*
      fprintf(stderr, ".DBM: STACK: %d\n", (int)d->value);
    */
  }
  else {
    snprintf(g_error_message, sizeof(g_error_message), ".%s cannot handle strings in \"_OUT/_out\".\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  return SUCCEEDED;
}


int macro_insert_word_db(char *name) {

  struct definition *d;
  
  if (hashmap_get(g_defines_map, "_out", (void*)&d) != MAP_OK)
    hashmap_get(g_defines_map, "_OUT", (void*)&d);

  if (d == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "No \"_OUT/_out\" defined, .%s takes its output from there.\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (d->type == DEFINITION_TYPE_VALUE) {
    if (d->value < -32768 || d->value > 65535) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s expects 16-bit data, %d is out of range!\n", name, (int)d->value);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    fprintf(g_file_out_ptr, "y%d ", (int)d->value);
    /*
      fprintf(stderr, ".DWM: VALUE: %d\n", (int)d->value);
    */
  }
  else if (d->type == DEFINITION_TYPE_STACK) {
    fprintf(g_file_out_ptr, "C%d ", (int)d->value);
    /*
      fprintf(stderr, ".DWM: STACK: %d\n", (int)d->value);
    */
  }
  else {
    snprintf(g_error_message, sizeof(g_error_message), ".%s cannot handle strings in \"_OUT/_out\".\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  return SUCCEEDED;
}


#if W65816

int macro_insert_long_db(char *name) {

  struct definition *d;
  
  if (hashmap_get(g_defines_map, "_out", (void*)&d) != MAP_OK)
    hashmap_get(g_defines_map, "_OUT", (void*)&d);

  if (d == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "No \"_OUT/_out\" defined, .%s takes its output from there.\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (d->type == DEFINITION_TYPE_VALUE) {
    if (d->value < -8388608 || d->value > 16777215) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s expects 24-bit data, %d is out of range!\n", name, (int)d->value);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    fprintf(g_file_out_ptr, "z%d ", (int)d->value);
    /*
      fprintf(stderr, ".DLM: VALUE: %d\n", (int)d->value);
    */
  }
  else if (d->type == DEFINITION_TYPE_STACK) {
    fprintf(g_file_out_ptr, "T%d ", (int)d->value);
    /*
      fprintf(stderr, ".DLM: STACK: %d\n", (int)d->value);
    */
  }
  else {
    snprintf(g_error_message, sizeof(g_error_message), ".%s cannot handle strings in \"_OUT/_out\".\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  return SUCCEEDED;
}

#endif


struct structure* get_structure(char *name) {

  struct structure *s = g_structures_first;

  while (s != NULL) {
    if (strcmp(name, s->name) == 0)
      return s;
    s = s->next;
  }

  return NULL;
}


int directive_define_def_equ(void);


int pass_1(void) {

  struct macro_runtime *mrt;
  struct macro_static *m = NULL;
  int o, p, q;
  
  if (g_verbose_mode == ON)
    printf("Pass 1...\n");

  /* mark all slots as empty */
  for (q = 0; q < 256; q++) {
    g_slots[q].size = 0;
    g_slots[q].name[0] = 0;
  }

  /* start from the very first character, this is the index to the source file we are about to parse... */
  g_source_pointer = 0;

  /* BANK 0 SLOT 0 ORG 0 */
  if (g_output_format != OUTPUT_LIBRARY)
    fprintf(g_file_out_ptr, "B%d %d O%d", 0, 0, 0);

  while (1) {
    t = get_next_token();
    if (t != SUCCEEDED)
      break;
    
    q = evaluate_token();

    if (q == SUCCEEDED)
      continue;
    else if (q == EVALUATE_TOKEN_EOP)
      return SUCCEEDED;
    else if (q == EVALUATE_TOKEN_NOT_IDENTIFIED) {
      /* check if it is of the form "LABEL:XYZ" */
      for (q = 0; q < g_ss; q++) {
        if (tmp[q] == ':')
          break;
        if (tmp[q] == '=')
          break;
      }

      /* is it a macro? */
      if (q == g_ss) {
        if (macro_get(tmp, YES, &m) == FAILED)
          return FAILED;
        if (m == NULL) {
          if (macro_get(tmp, NO, &m) == FAILED)
            return FAILED;
        }
      }

      /* it is a label after all? */
      if (q != g_ss || (g_newline_beginning == ON && m == NULL)) {
        char old_tmp_q = tmp[q];
    
        tmp[q] = 0;

        /* reset the flag as there can be only one label / line */
        g_newline_beginning = OFF;

        if (compare_next_token("=") == SUCCEEDED || old_tmp_q == '=') {
          /* it's actually a definition! */
          g_source_pointer -= g_ss;

          if (directive_define_def_equ() == FAILED)
            return FAILED;
        }
        else {
          if (g_output_format == OUTPUT_LIBRARY && g_section_status == OFF) {
            print_error("All labels must be inside sections when compiling a library.\n", ERROR_LOG);
            return FAILED;
          }
          if (g_org_defined == 0) {
            snprintf(g_error_message, sizeof(g_error_message), "\"%s\" needs a position in memory.\n", tmp);
            print_error(g_error_message, ERROR_LOG);
            return FAILED;
          }
          if (g_ss >= MAX_NAME_LENGTH) {
            snprintf(g_error_message, sizeof(g_error_message), "The label \"%s\" is too long. Max label length is %d characters.\n", tmp, MAX_NAME_LENGTH);
            print_error(g_error_message, ERROR_NONE);
            return FAILED;
          }
          if (g_bankheader_status == ON) {
            print_error("BANKHEADER sections don't take labels.\n", ERROR_LOG);
            return FAILED;
          }

          /* check out for \@-symbols */
          if (g_macro_active != 0 && q >= 2) {
            if (tmp[q - 2] == '\\' && tmp[q - 1] == '@')
              snprintf(&tmp[q - 2], sizeof(tmp) - (q - 2), "%d", g_macro_runtime_current->macro->calls - 1);
          }

          add_label_to_label_stack(tmp);
          fprintf(g_file_out_ptr, "k%d L%s ", g_active_file_info_last->line_current, tmp);

          /* move to the end of the label */
          if (q != g_ss)
            g_source_pointer -= g_ss - q - 1;
        }
    
        continue;
      }
      else if (compare_next_token("=") == SUCCEEDED) {
        /* it's actually a definition! */
        g_source_pointer -= g_ss;

        if (directive_define_def_equ() == FAILED)
          return FAILED;

        continue;
      }
      
      if (m == NULL) {
        snprintf(g_error_message, sizeof(g_error_message), "Unknown symbol \"%s\".\n", tmp);
        print_error(g_error_message, ERROR_ERR);
        return FAILED;
      }

      /* start running a macro... run until .ENDM */
      if (macro_stack_grow() == FAILED)
        return FAILED;

      mrt = &g_macro_stack[g_macro_active];
      mrt->argument_data = NULL;

      /* collect macro arguments */
      for (p = 0; 1; p++) {
        /* take away the white space */
        while (1) {
          if (g_buffer[g_source_pointer] == ' ' || g_buffer[g_source_pointer] == ',')
            g_source_pointer++;
          else
            break;
        }

        o = g_source_pointer;
        q = input_number();
        if (q == INPUT_NUMBER_EOL)
          break;

        mrt->argument_data = realloc(mrt->argument_data, (p+1)*sizeof(struct macro_argument *));
        mrt->argument_data[p] = calloc(sizeof(struct macro_argument), 1);
        if (mrt->argument_data == NULL || mrt->argument_data[p] == NULL) {
          print_error("Out of memory error while collecting macro arguments.\n", ERROR_NONE);
          return FAILED;
        }

        mrt->argument_data[p]->start = o;
        mrt->argument_data[p]->type = q;

        if (q == INPUT_NUMBER_ADDRESS_LABEL)
          strcpy(mrt->argument_data[p]->string, g_label);
        else if (q == INPUT_NUMBER_STRING)
          strcpy(mrt->argument_data[p]->string, g_label);
        else if (q == INPUT_NUMBER_STACK)
          mrt->argument_data[p]->value = (double)g_latest_stack;
        else if (q == SUCCEEDED)
          mrt->argument_data[p]->value = g_parsed_double;
        else
          return FAILED;

        /* do we have a name for this argument? */
        if (p < m->nargument_names) {
          if (q == INPUT_NUMBER_ADDRESS_LABEL)
            redefine(m->argument_names[p], 0.0, g_label, DEFINITION_TYPE_ADDRESS_LABEL, (int)strlen(g_label));
          else if (q == INPUT_NUMBER_STRING)
            redefine(m->argument_names[p], 0.0, g_label, DEFINITION_TYPE_STRING, (int)strlen(g_label));
          else if (q == INPUT_NUMBER_STACK)
            redefine(m->argument_names[p], (double)g_latest_stack, NULL, DEFINITION_TYPE_STACK, 0);
          else if (q == SUCCEEDED)
            redefine(m->argument_names[p], g_parsed_double, NULL, DEFINITION_TYPE_VALUE, 0);
        }
      }

      next_line();

      mrt->supplied_arguments = p;
      if (macro_start(m, mrt, MACRO_CALLER_NORMAL, p) == FAILED)
        return FAILED;

      continue;
    }
    else if (q == FAILED) {
      snprintf(g_error_message, sizeof(g_error_message), "Couldn't parse \"%s\".\n", tmp);
      print_error(g_error_message, ERROR_ERR);
      return FAILED;
    }
    else {
      printf("PASS_1: Internal error, unknown return type %d.\n", q);
      return FAILED;
    }
  }

  return FAILED;
}


void output_assembled_opcode(struct optcode *oc, const char *format, ...) {

  va_list ap;
  
  if (oc == NULL)
    return;
  
  va_start(ap, format);

  vfprintf(g_file_out_ptr, format, ap);
#ifdef WLA_DEBUG
  {
    char ttt[256];

    va_end(ap);
    va_start(ap, format);
    vsnprintf(ttt, sizeof(ttt), format, ap);
    printf("LINE %5d: OPCODE: %16s ::: %s\n", g_active_file_info_last->line_current, oc->op, ttt);
  }
#endif

  va_end(ap);
}


#if MC6809
static char error_no_u[] = "Was expecting register X/Y/S/PC/A/B/CC/DP";
static char error_no_s[] = "Was expecting register X/Y/U/PC/A/B/CC/DP";

static int parse_push_pull_registers(int accept_u) {

  int register_byte = 0, reg, y, z, prev_i;

  while (1) {
    y = g_source_pointer;
    g_source_pointer = inz;
    prev_i = g_source_pointer;
    z = input_number();
    inz = g_source_pointer;
    g_source_pointer = y;

    if (z == INPUT_NUMBER_EOL) {
      /* roll back to the index before EOL */
      inz = prev_i;
      break;
    }
    
    if (z != INPUT_NUMBER_ADDRESS_LABEL) {
      if (accept_u == 1) {
        snprintf(g_error_message, sizeof(g_error_message), "%s, got something else instead.\n", error_no_s);
        print_error(g_error_message, ERROR_ERR);
      }
      else {
        snprintf(g_error_message, sizeof(g_error_message), "%s, got something else instead.\n", error_no_u);
        print_error(g_error_message, ERROR_ERR);
      }
      return -1;
    }

    if (strcaselesscmp(g_label, "X") == 0)
      reg = 1 << 4;
    else if (strcaselesscmp(g_label, "Y") == 0)
      reg = 1 << 5;
    else if (strcaselesscmp(g_label, "U") == 0) {
      if (accept_u == 0) {
        snprintf(g_error_message, sizeof(g_error_message), "%s, got \"%s\" instead.\n", error_no_u, g_label);
        print_error(g_error_message, ERROR_ERR);
        return -1;
      }
      reg = 1 << 6;
    }
    else if (strcaselesscmp(g_label, "S") == 0) {
      if (accept_u == 1) {
        snprintf(g_error_message, sizeof(g_error_message), "%s, got \"%s\" instead.\n", error_no_s, g_label);
        print_error(g_error_message, ERROR_ERR);
        return -1;
      }
      reg = 1 << 6;
    }
    else if (strcaselesscmp(g_label, "PC") == 0)
      reg = 1 << 7;
    else if (strcaselesscmp(g_label, "A") == 0)
      reg = 1 << 1;
    else if (strcaselesscmp(g_label, "B") == 0)
      reg = 1 << 2;
    else if (strcaselesscmp(g_label, "CC") == 0)
      reg = 1 << 0;
    else if (strcaselesscmp(g_label, "DP") == 0)
      reg = 1 << 3;
    else {
      if (accept_u == 1) {
        snprintf(g_error_message, sizeof(g_error_message), "%s, got \"%s\" instead.\n", error_no_s, g_label);
        print_error(g_error_message, ERROR_ERR);
      }
      else {
        snprintf(g_error_message, sizeof(g_error_message), "%s, got \"%s\" instead.\n", error_no_u, g_label);
        print_error(g_error_message, ERROR_ERR);
      }
      return -1;
    }

    if ((register_byte & reg) != 0) {
      snprintf(g_error_message, sizeof(g_error_message), "Register \"%s\" was already defined.\n", g_label);
      print_error(g_error_message, ERROR_ERR);
      return -1;
    }

    register_byte |= reg;
  }

  if (register_byte == 0) {
    if (accept_u == 1)
      snprintf(g_error_message, sizeof(g_error_message), "%s, got nothing instead.\n", error_no_s);
    else
      snprintf(g_error_message, sizeof(g_error_message), "%s, got nothing instead.\n", error_no_u);
    print_error(g_error_message, ERROR_ERR);

    return -1;
  }
  
  return register_byte;
}


static int get_register_byte_from_label_exg_tfr() {

  /* 16-bit */
  if (strcaselesscmp(g_label, "D") == 0)
    return 0;
  if (strcaselesscmp(g_label, "X") == 0)
    return 1;
  if (strcaselesscmp(g_label, "Y") == 0)
    return 2;
  if (strcaselesscmp(g_label, "U") == 0)
    return 3;
  if (strcaselesscmp(g_label, "S") == 0)
    return 4;
  if (strcaselesscmp(g_label, "PC") == 0)
    return 5;

  /* 8-bit */
  if (strcaselesscmp(g_label, "A") == 0)
    return 8;
  if (strcaselesscmp(g_label, "B") == 0)
    return 9;
  if (strcaselesscmp(g_label, "CC") == 0)
    return 0xA;
  if (strcaselesscmp(g_label, "DP") == 0)
    return 0xB;

  snprintf(g_error_message, sizeof(g_error_message), "Was expecting register D/X/Y/U/S/PC/A/B/CC/DP, got \"%s\" instead.\n", g_label);
  print_error(g_error_message, ERROR_ERR);

  return -1;
}


static int parse_exg_tfr_registers() {

  int register_byte = 0, data = 0, y, z;

  /* source register */
  y = g_source_pointer;
  g_source_pointer = inz;
  z = input_number();
  inz = g_source_pointer;
  g_source_pointer = y;

  if (z != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error("Was expecting register D/X/Y/U/S/PC/A/B/CC/DP, got something else instead.\n", ERROR_ERR);
    return -1;
  }

  data = get_register_byte_from_label_exg_tfr();
  if (data < 0)
    return -1;
  register_byte = data;

  /* destination register */
  y = g_source_pointer;
  g_source_pointer = inz;
  z = input_number();
  inz = g_source_pointer;
  g_source_pointer = y;

  if (z != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error("Was expecting register D/X/Y/U/S/PC/A/B/CC/DP, got something else instead.\n", ERROR_ERR);
    return -1;
  }

  data = get_register_byte_from_label_exg_tfr();
  if (data < 0)
    return -1;

  /* are we mixing 16-bit and 8-bit registers? */
  if ((register_byte <= 5 && data > 5) || (register_byte > 5 && data <= 5)) {
    print_error("Mixing of 8-bit and 16-bit registers is not allowed here.\n", ERROR_ERR);
    return -1;    
  }
  
  register_byte = (register_byte << 4) | data;
  
  return register_byte;
}
#endif


int evaluate_token(void) {

  int f, z, x, y;
#if defined(Z80) || defined(SPC700) || defined(W65816) || defined(WDC65C02) || defined(CSG65CE02) || defined(HUC6280)
  int e = 0, v = 0, h = 0;
  char labelx[MAX_NAME_LENGTH + 1];
#endif
#ifdef SPC700
  int g;
#endif
#ifdef HUC6280
  int r = 0, s, t = 0, u = 0;
  char labely[MAX_NAME_LENGTH + 1];
#endif

  /* are we in an enum, ramsection, or struct? */
  if (in_enum == YES || in_ramsection == YES || in_struct == YES)
    return parse_enum_token();

  /* is it a directive? */
  if (tmp[0] == '.') {
    x = parse_directive();
    if (x != DIRECTIVE_NOT_IDENTIFIED)
      return x;

    /* allow error messages from input_numbers() */
    g_input_number_error_msg = YES;

    return EVALUATE_TOKEN_NOT_IDENTIFIED;
  }

  /* is it a label? */
  if (tmp[g_ss - 1] == ':' && g_newline_beginning == ON) {
    tmp[g_ss - 1] = 0;
    g_newline_beginning = OFF;

    if (g_output_format == OUTPUT_LIBRARY && g_section_status == OFF) {
      print_error("All labels must be inside sections when compiling a library.\n", ERROR_LOG);
      return FAILED;
    }
    if (g_org_defined == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "\"%s\" needs a position in memory.\n", tmp);
      print_error(g_error_message, ERROR_LOG);
      return FAILED;
    }
    if (g_ss >= MAX_NAME_LENGTH) {
      snprintf(g_error_message, sizeof(g_error_message), "The label \"%s\" is too long. Max label length is %d characters.\n", tmp, MAX_NAME_LENGTH);
      print_error(g_error_message, ERROR_NONE);
      return FAILED;
    }
    if (g_bankheader_status == ON) {
      print_error("BANKHEADER sections don't take labels.\n", ERROR_LOG);
      return FAILED;
    }

    /* check for \@-symbols */
    if (g_macro_active != 0) {
      if (tmp[g_ss - 3] == '\\' && tmp[g_ss - 2] == '@')
        snprintf(&tmp[g_ss - 3], sizeof(tmp) - (g_ss - 3), "%d", g_macro_runtime_current->macro->calls - 1);
    }

    add_label_to_label_stack(tmp);
    fprintf(g_file_out_ptr, "k%d L%s ", g_active_file_info_last->line_current, tmp);

    return SUCCEEDED;
  }

  /* OPCODE? */
  {
    int op_id = tmp[0];

    if (op_id < 0) {
      print_error("Invalid value\n", ERROR_LOG);
      return FAILED;
    }

    ind = opcode_p[op_id];
  }

  g_opt_tmp = &opt_table[ind];

  for (f = opcode_n[(unsigned int)tmp[0]]; f > 0; f--) {
#if W65816
    if (g_use_wdc_standard == 0) {
      /* skip all mnemonics that contain '<', '|' and '>' */
      for (inz = 0, d = SUCCEEDED; inz < OP_SIZE_MAX; inz++) {
        char c = g_opt_tmp->op[inz];

        if (c == 0)
          break;
        if (c == '<' || c == '|' || c == '>') {
          d = FAILED;
          break;
        }
      }

      if (d == FAILED) {
        /* try the next mnemonic in the array */
        g_opt_tmp = &opt_table[++ind];
        continue;
      }
    }
#endif
    
    /* try to match the first part of the mnemonic, already read into tmp */
    for (inz = 0, d = SUCCEEDED; inz < OP_SIZE_MAX; inz++) {
      if (tmp[inz] == 0)
        break;
      if (g_opt_tmp->op[inz] != toupper((int)tmp[inz])) {
        d = FAILED;
        break;
      }
    }

    if (d == FAILED) {
      /* try the next mnemonic in the array */
      g_opt_tmp = &opt_table[++ind];
      continue;
    }

    /* beginning matches the input */
    x = inz;
    inz = g_source_pointer;

#ifndef GB
    /* no stack rollback */
    g_stack_inserted = STACK_NONE;
#endif

    switch (g_opt_tmp->type) {

#ifdef GB
#include "decode_gb.c"
#endif
#ifdef Z80
#include "decode_z80.c"
#endif
#ifdef MCS6502
#include "decode_6502.c"
#endif
#ifdef WDC65C02
#include "decode_65c02.c"
#endif
#ifdef CSG65CE02
#include "decode_65ce02.c"
#endif
#ifdef MCS6510
#include "decode_6510.c"
#endif
#ifdef W65816
#include "decode_65816.c"
#endif
#ifdef MC6800
#include "decode_6800.c"
#endif
#ifdef MC6801
#include "decode_6801.c"
#endif
#ifdef MC6809
#include "decode_6809.c"
#endif
#ifdef I8008
#include "decode_8008.c"
#endif
#ifdef I8080
#include "decode_8080.c"
#endif
#ifdef SPC700
#include "decode_spc700.c"
#endif
#ifdef HUC6280
#include "decode_huc6280.c"
#endif

    }

#ifndef GB
    /* perform stack rollback? */
    if (g_stack_inserted != STACK_NONE) {

      struct stack *s;

      if (g_stack_inserted == STACK_OUTSIDE) {
        if (g_stacks_outside == 1) {
          g_stacks_outside = 0;
          delete_stack(g_stacks_first);
          g_stacks_first = NULL;
          g_stacks_last = NULL;
        }
        else {
          s = g_stacks_first;
          g_stacks_outside--;

          for (y = 0; y < g_stacks_outside - 1; y++)
            s = s->next;

          delete_stack(s->next);
          s->next = NULL;
          g_stacks_last = s;
        }
      }
      else {
        if (g_stacks_inside == 1) {
          g_stacks_inside = 0;
          delete_stack(g_stacks_header_first);
          g_stacks_header_first = NULL;
          g_stacks_header_last = NULL;
        }
        else {
          s = g_stacks_header_first;
          g_stacks_inside--;

          for (y = 0; y < g_stacks_inside - 1; y++)
            s = s->next;

          delete_stack(s->next);
          s->next = NULL;
          g_stacks_header_last = s;
        }
      }
    }
#endif

    g_opt_tmp = &opt_table[++ind];
  }

  /* allow error messages from input_numbers() */
  g_input_number_error_msg = YES;

  return EVALUATE_TOKEN_NOT_IDENTIFIED;
}


int redefine(char *name, double value, char *string, int type, int size) {

  struct definition *d;
  
  hashmap_get(g_defines_map, name, (void*)&d);
  
  /* it wasn't defined previously */
  if (d == NULL)
    return add_a_new_definition(name, value, string, type, size);

  d->type = type;

  if (type == DEFINITION_TYPE_VALUE)
    d->value = value;
  else if (type == DEFINITION_TYPE_STACK)
    d->value = value;
  else if (type == DEFINITION_TYPE_STRING || type == DEFINITION_TYPE_ADDRESS_LABEL) {
    memcpy(d->string, string, size);
    d->string[size] = 0;
    d->size = size;
  }

  return SUCCEEDED;
}


int undefine(char *name) {

  struct definition *d;
  
  if (hashmap_get(g_defines_map, name, (void*)&d) != MAP_OK)
    return FAILED;

  hashmap_remove(g_defines_map, name);

  free(d);

  return SUCCEEDED;
}


int add_a_new_definition(char *name, double value, char *string, int type, int size) {

  struct definition *d;
  int err;

  /* we skip definitions for "." (because .ENUM and .RAMSECTION use it as an anonymous label) */
  if (strcmp(".", name) == 0 || strcmp("_sizeof_.", name) == 0)
    return SUCCEEDED;

  hashmap_get(g_defines_map, name, (void*)&d);
  if (d != NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "\"%s\" was defined for the second time.\n", name);
    if (g_commandline_parsing == OFF)
      print_error(g_error_message, ERROR_DIR);
    else
      fprintf(stderr, "ADD_A_NEW_DEFINITION: %s", g_error_message);
    return FAILED;
  }

  d = calloc(sizeof(struct definition), 1);
  if (d == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Out of memory while trying to add a new definition (\"%s\").\n", name);
    if (g_commandline_parsing == OFF)
      print_error(g_error_message, ERROR_DIR);
    else
      fprintf(stderr, "ADD_A_NEW_DEFINITION: %s", g_error_message);
    return FAILED;
  }

  strcpy(d->alias, name);
  d->type = type;

  if ((err = hashmap_put(g_defines_map, d->alias, d)) != MAP_OK) {
    fprintf(stderr, "ADD_A_NEW_DEFINITION: Hashmap error %d\n", err);
    return FAILED;
  }

  if (type == DEFINITION_TYPE_VALUE)
    d->value = value;
  else if (type == DEFINITION_TYPE_STACK)
    d->value = value;
  else if (type == DEFINITION_TYPE_STRING || type == DEFINITION_TYPE_ADDRESS_LABEL) {
    memcpy(d->string, string, size);
    d->string[size] = 0;
    d->size = size;
  }

  return SUCCEEDED;
}


int localize_path(char *path) {

  int i;
  
  if (path == NULL)
    return FAILED;

  for (i = 0; path[i] != 0; i++) {
#if defined(MSDOS)
    /* '/' -> '\' */
    if (path[g_source_pointer] == '/')
      path[g_source_pointer] = '\\';
#else
    /* '\' -> '/' */
    if (path[i] == '\\')
      path[i] = '/';
#endif
  }

  return SUCCEEDED;
}


int verify_name_length(char *name) {

  if (strlen(name) > MAX_NAME_LENGTH) {
    snprintf(g_error_message, sizeof(g_error_message), "The label \"%s\" is too long. Max label length is %d bytes.\n", name, MAX_NAME_LENGTH);
    print_error(g_error_message, ERROR_NONE);
    return FAILED;
  }

  return SUCCEEDED;
}


void print_error(char *error, int type) {

  char error_dir[] = "DIRECTIVE_ERROR:";
  char error_unf[] = "UNFOLD_ALIASES:";
  char error_num[] = "INPUT_NUMBER:";
  char error_inc[] = "INCLUDE_FILE:";
  char error_inb[] = "INCBIN_FILE:";
  char error_inp[] = "INPUT_ERROR:";
  char error_log[] = "LOGIC_ERROR:";
  char error_stc[] = "STACK_CALCULATE:";
  char error_wrn[] = "WARNING:";
  char error_err[] = "ERROR:";
  char *t = NULL;

  switch (type) {
  case ERROR_LOG:
    t = error_log;
    break;
  case ERROR_UNF:
    t = error_unf;
    break;
  case ERROR_INC:
    t = error_inc;
    break;
  case ERROR_INB:
    t = error_inb;
    break;
  case ERROR_DIR:
    t = error_dir;
    break;
  case ERROR_INP:
    t = error_inp;
    break;
  case ERROR_NUM:
    t = error_num;
    break;
  case ERROR_STC:
    t = error_stc;
    break;
  case ERROR_WRN:
    t = error_wrn;
    break;
  case ERROR_ERR:
    t = error_err;
    break;
  case ERROR_NONE:
    fprintf(stderr, "%s:%d: %s", get_file_name(g_active_file_info_last->filename_id), g_active_file_info_last->line_current, error);
    return;
  }

  fprintf(stderr, "%s:%d: %s %s", get_file_name(g_active_file_info_last->filename_id), g_active_file_info_last->line_current, t, error);
  fflush(stderr);

  return;
}


#ifdef W65816

void give_snes_rom_mode_defined_error(char *prior) {

  if (g_lorom_defined != 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".LOROM was defined prior to %s.\n", prior);
    print_error(g_error_message, ERROR_DIR);
  }
  else if (g_hirom_defined != 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".HIROM was defined prior to %s.\n", prior);
    print_error(g_error_message, ERROR_DIR);
  }
  else if (g_exlorom_defined != 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".EXLOROM was defined prior to %s.\n", prior);
    print_error(g_error_message, ERROR_DIR);
  }
  else if (g_exhirom_defined != 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".EXHIROM was defined prior to %s.\n", prior);
    print_error(g_error_message, ERROR_DIR);
  }
}

#endif


void next_line(void) {

  g_newline_beginning = ON;

  if (g_line_count_status == OFF)
    return;

  if (g_active_file_info_last == NULL)
    return;

  /* output the file number for list file structure building */
  if (g_listfile_data == YES)
    fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  if (g_active_file_info_last != NULL)
    g_active_file_info_last->line_current++;
}


/* used by .RAMSECTIONs only */
int add_label_sizeof(char *label, int g_size) {

  struct label_sizeof *ls;
  char tmpname[MAX_NAME_LENGTH + 8];

  if (g_create_sizeof_definitions == NO)
    return SUCCEEDED;
  
  /* we skip definitions for '_sizeof_.' (because .ENUM and .RAMSECTION use it as an anonymous label) */
  if (strcmp(".", label) == 0)
    return SUCCEEDED;

  ls = calloc(sizeof(struct label_sizeof), 1);
  if (ls == NULL) {
    print_error("Out of memory error while allocating a label sizeof structure.\n", ERROR_DIR);
    return FAILED;
  }
  
  strcpy(ls->name, label);
  ls->size = g_size;
  ls->next = g_label_sizeofs;
  g_label_sizeofs = ls;

  /* define locally also, since we can */
  snprintf(tmpname, sizeof(tmpname), "_sizeof_%s", label);
  if (add_a_new_definition(tmpname, (double)g_size, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


void free_struct(struct structure *st) {

  struct structure_item *si = st->items;

  while (si != NULL) {
    struct structure_item *tmp = si;

    if (si->type == STRUCTURE_ITEM_TYPE_UNION)
      free_struct(si->union_items);
    /* don't free si->instance for STRUCTURE_ITEM_TYPE_INSTANCE since that's a reusable
       structure */
    si = si->next;
    free(tmp);
  }

  free(st);
}


/* enum_offset and last_enum_offset should be set when calling this. */
int add_label_to_enum_or_ramsection(char *name, int size) {

  char tmp[MAX_NAME_LENGTH+10];

  /* there are two passes done when adding a temporary struct to an enum/ramsection. first
     pass is to add the labels, second is to add sizeof definitions. if done in only one
     pass, the resulting sym file is very ugly... */
  if (enum_sizeof_pass == NO) {
    if (verify_name_length(name) == FAILED)
      return FAILED;

    if (in_enum || in_struct) {
      if (add_a_new_definition(name, (double)(base_enum_offset+enum_offset), NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
        return FAILED;
      if (enum_exp == YES)
        if (export_a_definition(name) == FAILED)
          return FAILED;
    }
    else if (in_ramsection) {
      if (last_enum_offset != enum_offset) {
        /* this sometimes abuses the "dsb" implementation to move backwards in the ramsection. */
        fprintf(g_file_out_ptr, "x%d 0 ", enum_offset-last_enum_offset);
      }
      fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);
      /* we skip label emissions for "." (because .ENUM and .RAMSECTION use it as an anonymous label) */
      if (strcmp(".", name) != 0)
        fprintf(g_file_out_ptr, "L%s ", name);
    }
  }
  else { /* sizeof pass */
    if (in_ramsection) {
      if (add_label_sizeof(name, size) == FAILED)
        return FAILED;
    }
    else {
      if (g_create_sizeof_definitions == YES) {
        snprintf(tmp, sizeof(tmp), "_sizeof_%s", name);
        if (add_a_new_definition(tmp, (double)size, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
          return FAILED;
        if (in_enum == YES && enum_exp == YES) {
          if (export_a_definition(tmp) == FAILED)
            return FAILED;
        }
      }
    }
  }

  last_enum_offset = enum_offset;

  return SUCCEEDED;
}


/* add all fields from a struct at the current offset in the enum/ramsection.
   this is used to construct enums or ramsections through temporary structs, even if
   INSTANCEOF isn't used. enum_sizeof_pass should be set to YES or NO before calling. */
int enum_add_struct_fields(char *basename, struct structure *st, int reverse) {

  char tmp[MAX_NAME_LENGTH * 2 + 5];
  struct structure_item *si;
  int real_si_size, g;

  if (strlen(basename) > MAX_NAME_LENGTH) {
    snprintf(g_error_message, sizeof(g_error_message), "Name \"%s\" is too long!\n", basename);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  si = st->items;
  while (si != NULL) {
    real_si_size = si->size;
    if (si->type == STRUCTURE_ITEM_TYPE_DOTTED)
      real_si_size = 0;

    if (reverse)
      enum_offset -= real_si_size;

    /* make definition for this item */
    if (si->name[0] != '\0') {
      if (basename[0] != '\0')
        snprintf(tmp, sizeof(tmp), "%s.%s", basename, si->name);
      else
        snprintf(tmp, sizeof(tmp), "%s", si->name);

      if (verify_name_length(tmp) == FAILED)
        return FAILED;

      if (add_label_to_enum_or_ramsection(tmp, si->size) == FAILED)
        return FAILED;
    }

    /* if this struct has an .instanceof in it, we need to recurse */
    if (si->type == STRUCTURE_ITEM_TYPE_INSTANCEOF) {
      if (si->num_instances <= 1) {
        /* add definition for first (possibly only) instance of struct */
        if (enum_add_struct_fields(tmp, si->instance, 0) == FAILED)
          return FAILED;
      }
      else {
        g = si->start_from;
        while (g < si->start_from + si->num_instances) {
          if (basename[0] != '\0')
            snprintf(tmp, sizeof(tmp), "%s.%s.%d", basename, si->name, g);
          else
            snprintf(tmp, sizeof(tmp), "%s.%d", si->name, g);

          if (verify_name_length(tmp) == FAILED)
            return FAILED;
          if (add_label_to_enum_or_ramsection(tmp, si->instance->size) == FAILED)
            return FAILED;
          if (enum_add_struct_fields(tmp, si->instance, 0) == FAILED)
            return FAILED;
          g++;
        }
      }
    }
    /* if this struct has a .union in it, we treat each union block like a struct */
    else if (si->type == STRUCTURE_ITEM_TYPE_UNION) {
      int orig_offset = enum_offset;
      char union_basename[MAX_NAME_LENGTH * 2 + 5];
      struct structure *un = si->union_items;

      while (un != NULL) {
        enum_offset = orig_offset;

        if (un->name[0] != '\0') {
          if (basename[0] != '\0')
            snprintf(union_basename, sizeof(union_basename), "%s.%s", basename, un->name);
          else
            snprintf(union_basename, sizeof(union_basename), "%s", un->name);

          if (verify_name_length(union_basename) == FAILED)
            return FAILED;

          if (add_label_to_enum_or_ramsection(union_basename, un->size) == FAILED)
            return FAILED;
        }
        else
          snprintf(union_basename, sizeof(union_basename), "%s", basename);

        if (enum_add_struct_fields(union_basename, un, 0) == FAILED)
          return FAILED;

        un = un->next;
      }

      /* this is the size of the largest union */
      enum_offset = orig_offset + real_si_size;
    }
    else
      enum_offset += real_si_size;

    if (reverse) /* after defining data, go back to start, for DESC enums */
      enum_offset -= real_si_size;

    si = si->next;
  }

  return SUCCEEDED;
}


/* either "in_enum", "in_ramsection", or "in_struct" should be true when this is called. */
int parse_enum_token(void) {

  struct structure *st = NULL;
  struct structure_item *si;
  char tmpname[MAX_NAME_LENGTH + 8 + 1], bak[256];
  int type, g_size, q, start_from = 1;
  
  /* check for "if" directives (the only directives permitted in an enum/ramsection) */
  if (tmp[0] == '.') {
    if ((q = parse_if_directive()) != -1) {
      return q;
    }
  }

  if (strcaselesscmp(tmp, ".UNION") == 0) {
    struct union_stack *ust;
    int inz;

    st = calloc(sizeof(struct structure), 1);
    if (st == NULL) {
      print_error("PARSE_ENUM_TOKEN: Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }
    st->items = NULL;
    st->last_item = NULL;

    inz = input_next_string();
    if (inz == FAILED) {
      free(st);
      return FAILED;
    }
    else if (inz == SUCCEEDED)
      strcpy(st->name, tmp);
    else
      st->name[0] = '\0';

    /* put previous union onto the "stack" */
    ust = calloc(sizeof(struct union_stack), 1);
    if (ust == NULL) {
      print_error("PARSE_ENUM_TOKEN: Out of memory error.\n", ERROR_DIR);
      free(st);
      return FAILED;
    }
    ust->active_struct = active_struct;
    ust->union_first_struct = union_first_struct;
    ust->union_base_offset = union_base_offset;
    ust->max_enum_offset = max_enum_offset;
    ust->prev = union_stack;
    union_stack = ust;

    active_struct = st;
    union_first_struct = active_struct;
    union_base_offset = enum_offset;
    max_enum_offset = union_base_offset;
    return SUCCEEDED;
  }
  else if (strcaselesscmp(tmp, ".NEXTU") == 0) {
    int inz;

    if (union_stack == NULL) {
      print_error("There is no open union.\n", ERROR_DIR);
      return FAILED;
    }

    if (enum_offset > max_enum_offset)
      max_enum_offset = enum_offset;
    active_struct->size = enum_offset - union_base_offset;
    st = calloc(sizeof(struct structure), 1);
    if (st == NULL) {
      print_error("PARSE_ENUM_TOKEN: Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }
    st->items = NULL;
    st->last_item = NULL;
    
    inz = input_next_string();
    if (inz == FAILED) {
      free(st);
      return FAILED;
    }
    else if (inz == SUCCEEDED)
      strcpy(st->name, tmp);
    else
      st->name[0] = '\0';

    active_struct->next = st;
    active_struct = st;
    enum_offset = union_base_offset;
    return SUCCEEDED;
  }
  else if (strcaselesscmp(tmp, ".ENDU") == 0) {
    struct union_stack *ust;
    int total_size;

    if (union_stack == NULL) {
      print_error("There is no open union.\n", ERROR_DIR);
      return FAILED;
    }

    if (enum_offset > max_enum_offset)
      max_enum_offset = enum_offset;

    total_size = max_enum_offset - union_base_offset;

    active_struct->size = enum_offset - union_base_offset;
    active_struct->next = NULL;

    st = union_first_struct;

    enum_offset = max_enum_offset;

    /* pop previous union from the "stack" */
    ust = union_stack;
    active_struct = union_stack->active_struct;
    union_first_struct = union_stack->union_first_struct;
    union_base_offset = ust->union_base_offset;
    max_enum_offset = ust->max_enum_offset;
    union_stack = union_stack->prev;
    free(ust);

    /* just popped max_enum_offset; need to update it for end of union */
    if (enum_offset > max_enum_offset)
      max_enum_offset = enum_offset;

    /* create a new structure item of type STRUCTURE_ITEM_TYPE_UNION */
    si = calloc(sizeof(struct structure_item), 1);
    if (si == NULL) {
      print_error("PARSE_ENUM_TOKEN: Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }
    si->name[0] = '\0';
    si->type = STRUCTURE_ITEM_TYPE_UNION;
    si->size = total_size;
    si->next = NULL;
    si->union_items = st;
    if (active_struct->items == NULL)
      active_struct->items = si;
    if (active_struct->last_item != NULL)
      active_struct->last_item->next = si;
    active_struct->last_item = si;
    return SUCCEEDED;
  }
  else if (in_enum == YES && strcaselesscmp(tmp, ".ENDE") == 0) {
    if (union_stack != NULL) {
      print_error("Union not closed.\n", ERROR_DIR);
      return FAILED;
    }
    
    enum_offset = 0;
    enum_sizeof_pass = NO;
    if (enum_add_struct_fields("", active_struct, (enum_ord == -1 ? 1 : 0)) == FAILED)
      return FAILED;

    enum_offset = 0;
    enum_sizeof_pass = YES;
    if (enum_add_struct_fields("", active_struct, (enum_ord == -1 ? 1 : 0)) == FAILED)
      return FAILED;

    free_struct(active_struct);
    active_struct = NULL;

    in_enum = NO;
    return SUCCEEDED;
  }
  else if (in_ramsection == YES && strcaselesscmp(tmp, ".ENDS") == 0) {
    if (union_stack != NULL) {
      print_error("Union not closed.\n", ERROR_DIR);
      return FAILED;
    }

    enum_offset = 0;
    last_enum_offset = 0;
    enum_sizeof_pass = NO;
    if (enum_add_struct_fields("", active_struct, 0) == FAILED)
      return FAILED;

    enum_offset = 0;
    last_enum_offset = 0;
    enum_sizeof_pass = YES;
    if (enum_add_struct_fields("", active_struct, 0) == FAILED)
      return FAILED;

    if (max_enum_offset > last_enum_offset)
      fprintf(g_file_out_ptr, "o%d 0 ", max_enum_offset-last_enum_offset);

    /* generate a section end label? */
    if (g_extra_definitions == ON)
      generate_label("SECTIONEND_", g_sections_last->name);
    
    free_struct(active_struct);
    active_struct = NULL;

    fprintf(g_file_out_ptr, "s ");
    g_section_status = OFF;
    in_ramsection = NO;

    return SUCCEEDED;
  }
  else if (in_struct && strcaselesscmp(tmp, ".ENDST") == 0) {
    enum_offset = 0;
    last_enum_offset = 0;
    enum_sizeof_pass = NO;
    if (enum_add_struct_fields(active_struct->name, active_struct, 0) == FAILED)
      return FAILED;

    enum_offset = 0;
    last_enum_offset = 0;
    enum_sizeof_pass = YES;
    if (enum_add_struct_fields(active_struct->name, active_struct, 0) == FAILED)
      return FAILED;
    
    /* create the SIZEOF-definition for the entire struct */
    active_struct->size = max_enum_offset;

    if (g_create_sizeof_definitions == YES) {
      if (strlen(active_struct->name) > MAX_NAME_LENGTH - 8) {
        snprintf(g_error_message, sizeof(g_error_message), "STRUCT \"%s\"'s name is too long!\n", active_struct->name);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      snprintf(tmpname, sizeof(tmpname), "_sizeof_%s", active_struct->name);
      if (add_a_new_definition(tmpname, (double)active_struct->size, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
        return FAILED;
    }
    
    if (active_struct->items == NULL) {
      snprintf(g_error_message, sizeof(g_error_message), "STRUCT \"%s\" is empty!\n", active_struct->name);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    active_struct->next = g_structures_first;
    g_structures_first = active_struct;

    in_struct = NO;
    active_struct = NULL;
    return SUCCEEDED;
  }

  if (tmp[strlen(tmp) - 1] == ':')
    tmp[strlen(tmp) - 1] = 0;
  strcpy(tmpname, tmp);

  /* get the size/type */
  if (get_next_token() == FAILED)
    return FAILED;
    
  type = 0;
  g_size = 0;

  if (strcaselesscmp(tmp, "DB") == 0 || strcaselesscmp(tmp, "BYT") == 0 || strcaselesscmp(tmp, "BYTE") == 0) {
    g_size = 1;
    type = STRUCTURE_ITEM_TYPE_DATA;
  }
  else if (strcaselesscmp(tmp, "DW") == 0 || strcaselesscmp(tmp, "WORD") == 0 || strcaselesscmp(tmp, "ADDR") == 0) {
    g_size = 2;
    type = STRUCTURE_ITEM_TYPE_DATA;
  }
#ifdef W65816
  else if (strcaselesscmp(tmp, "DL") == 0 || strcaselesscmp(tmp, "LONG") == 0 || strcaselesscmp(tmp, "FARADDR") == 0) {
    g_size = 3;
    type = STRUCTURE_ITEM_TYPE_DATA;
  }
#endif
  else if (strcaselesscmp(tmp, "DS") == 0 || strcaselesscmp(tmp, "DSB") == 0) {
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error("DS/DSB needs size.\n", ERROR_DIR);
      return FAILED;
    }
    g_size = d;
    type = STRUCTURE_ITEM_TYPE_DATA;
  }
  else if (strcaselesscmp(tmp, "DSW") == 0) {
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error("DSW needs size.\n", ERROR_DIR);
      return FAILED;
    }
    g_size = 2*d;
    type = STRUCTURE_ITEM_TYPE_DATA;
  }
#ifdef W65816
  else if (strcaselesscmp(tmp, "DSL") == 0) {
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error("DSL needs size.\n", ERROR_DIR);
      return FAILED;
    }
    g_size = 3*d;
    type = STRUCTURE_ITEM_TYPE_DATA;
  }
#endif
  /* it's an instance of a structure! */
  else if (strcaselesscmp(tmp, "INSTANCEOF") == 0) {
    type = STRUCTURE_ITEM_TYPE_INSTANCEOF;

    if (get_next_token() == FAILED)
      return FAILED;

    st = get_structure(tmp);

    if (st == NULL) {
      snprintf(g_error_message, sizeof(g_error_message), "No STRUCT named \"%s\" available.\n", tmp);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    /* get the number of structures to be made */
    inz = input_number();
    if (inz == INPUT_NUMBER_EOL) {
      next_line();
      g_size = st->size;
      d = 1;
    }
    else if (inz == SUCCEEDED) {
      if (d < 1) {
        print_error("The number of structures must be greater than 0.\n", ERROR_DIR);
        return FAILED;
      }

      g_size = st->size * d;
    }
    else {
      if (inz == INPUT_NUMBER_STRING)
        snprintf(g_error_message, sizeof(g_error_message), "Expected the number of structures, got \"%s\" instead.\n", g_label);
      else
        snprintf(g_error_message, sizeof(g_error_message), "Expected the number of structures.\n");
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (compare_next_token("STARTFROM") == SUCCEEDED) {
      skip_next_token();

      q = input_number();

      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED) {
        if (d < 0) {
          snprintf(g_error_message, sizeof(g_error_message), "STARTFROM needs to be >= 0.\n");
          print_error(g_error_message, ERROR_DIR);
          return FAILED;
        }
        start_from = d;
      }
      else {
        snprintf(g_error_message, sizeof(g_error_message), "STARTFROM needs a number >= 0.\n");
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
    }
  }
  else if (strcaselesscmp(tmp, ".db") == 0 || strcaselesscmp(tmp, ".byt") == 0 ||
           strcaselesscmp(tmp, ".byte") == 0) {
    /* don't do anything for "dotted" versions */
    g_size = 1;
    type = STRUCTURE_ITEM_TYPE_DOTTED;
  }
  else if (strcaselesscmp(tmp, ".dw") == 0 || strcaselesscmp(tmp, ".word") == 0 ||
           strcaselesscmp(tmp, ".addr") == 0) {
    /* don't do anything for "dotted" versions */
    g_size = 2;
    type = STRUCTURE_ITEM_TYPE_DOTTED;
  }
  else if (strcaselesscmp(tmp, ".ds") == 0 || strcaselesscmp(tmp, ".dsb") == 0 || strcaselesscmp(tmp, ".dsw") == 0) {
    /* don't do anything for "dotted" versions */
    strcpy(bak, tmp);
    
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      snprintf(g_error_message, sizeof(g_error_message), "%s needs size.\n", bak);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (strcaselesscmp(bak, ".dsw") == 0)
      d *= 2;

    g_size = d;
    type = STRUCTURE_ITEM_TYPE_DOTTED;
  }
#ifdef W65816
  else if (strcaselesscmp(tmp, ".dl") == 0 || strcaselesscmp(tmp, ".long") == 0 || strcaselesscmp(tmp, ".faraddr") == 0) {
    /* don't do anything for "dotted" versions */
    g_size = 3;
    type = STRUCTURE_ITEM_TYPE_DOTTED;
  }
  else if (strcaselesscmp(tmp, ".dsl") == 0) {
    /* don't do anything for "dotted" versions */
    strcpy(bak, tmp);
    
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error(".DSL needs size.\n", ERROR_DIR);
      return FAILED;
    }

    g_size = d * 3;
    type = STRUCTURE_ITEM_TYPE_DOTTED;
  }
#endif
  else {
    if (in_enum == YES)
      snprintf(g_error_message, sizeof(g_error_message), "Unexpected symbol \"%s\" in .ENUM.\n", tmp);
    else if (in_ramsection == YES)
      snprintf(g_error_message, sizeof(g_error_message), "Unexpected symbol \"%s\" in .RAMSECTION.\n", tmp);
    else /* struct */
      snprintf(g_error_message, sizeof(g_error_message), "Unexpected symbol \"%s\" in .STRUCT.\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* add this label/value to the struct. */
  si = calloc(sizeof(struct structure_item), 1);
  if (si == NULL) {
    print_error("Out of memory while allocating a new STRUCT.\n", ERROR_DIR);
    return FAILED;
  }
  si->next = NULL;
  strcpy(si->name, tmpname);
  si->size = g_size;
  si->type = type;
  si->start_from = start_from;
  if (type == STRUCTURE_ITEM_TYPE_INSTANCEOF) {
    si->instance = st;
    si->num_instances = si->size/st->size;
  }
  else if (type == STRUCTURE_ITEM_TYPE_UNION)
    si->union_items = st;

  if (active_struct->items == NULL)
    active_struct->items = si;
  if (active_struct->last_item != NULL)
    active_struct->last_item->next = si;
  active_struct->last_item = si;

  if (type != STRUCTURE_ITEM_TYPE_DOTTED)
    enum_offset += g_size;

  if (enum_offset > max_enum_offset)
    max_enum_offset = enum_offset;

  return SUCCEEDED;
}


int directive_org(void) {

  int q;
  
  no_library_files(".ORG definitions");
    
  if (g_bank_defined == 0) {
    print_error("No .BANK is defined.\n", ERROR_LOG);
    return FAILED;
  }
  if (g_section_status == ON) {
    print_error("You can't issue .ORG inside a .SECTION.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_dstruct_status == ON) {
    print_error("You can't issue .ORGA inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  q = input_number();

  if (q == FAILED)
    return FAILED;

  if (q != SUCCEEDED) {
    print_error(".ORG needs a positive or zero integer value.\n", ERROR_DIR);
    return FAILED;
  }

  g_org_defined = 1;
  fprintf(g_file_out_ptr, "O%d ", d);

  return SUCCEEDED;
}


int directive_orga(void) {

  int q;
  
  no_library_files(".ORGA definitions");
    
  if (g_bank_defined == 0) {
    print_error("No .BANK is defined.\n", ERROR_LOG);
    return FAILED;
  }
  if (g_section_status == ON) {
    print_error("You can't issue .ORGA inside a .SECTION.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_dstruct_status == ON) {
    print_error("You can't issue .ORGA inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  q = input_number();

  if (q == FAILED)
    return FAILED;

  if (q != SUCCEEDED) {
    print_error(".ORGA needs a positive or zero integer value.\n", ERROR_DIR);
    return FAILED;
  }

  g_org_defined = 1;

  ind = g_slots[g_current_slot].address;
  if (d < ind || d > (ind + g_slots[g_current_slot].size)) {
    print_error(".ORGA is outside the current SLOT.\n", ERROR_DIR);
    return FAILED;
  }

  fprintf(g_file_out_ptr, "O%d ", d - ind);

  return SUCCEEDED;
}


int directive_slot(void) {

  int q;
  
  no_library_files(".SLOT definitions");
    
  if (g_section_status == ON) {
    print_error("You can't issue .SLOT inside a .SECTION.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_dstruct_status == ON) {
    print_error("You can't issue .SLOT inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  q = input_number();

  if (q == INPUT_NUMBER_STRING || q == INPUT_NUMBER_ADDRESS_LABEL) {
    /* turn the label into a number */
    if (_get_slot_number_by_its_name(g_label, &d) == FAILED)
      return FAILED;
    q = SUCCEEDED;
  }
  else if (q == SUCCEEDED) {
    /* is the number a direct SLOT number, or is it an address? */
    q = _get_slot_number_by_a_value(d, &d);
  }
  if (q == FAILED)
    return FAILED;

  if (q != SUCCEEDED) {
    print_error("Cannot find the SLOT.\n", ERROR_DIR);
    return FAILED;
  }

  if (d < 0 || d > 255 || g_slots[d].size == 0) {
    snprintf(g_error_message, sizeof(g_error_message), "There is no SLOT number %d.\n", d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  fprintf(g_file_out_ptr, "B%d %d ", g_bank, d);

  g_current_slot = d;

  return SUCCEEDED;
}


int directive_bank(void) {

  int q;
  
  no_library_files(".BANK definitions");
    
  if (g_section_status == ON) {
    snprintf(g_error_message, sizeof(g_error_message), "Section \"%s\" is open. Do not try to change the bank.\n", g_sections_last->name);
    print_error(g_error_message, ERROR_LOG);
    return FAILED;
  }
  if (g_dstruct_status == ON) {
    print_error("You can't use .BANK inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_rombanks_defined == 0 && g_output_format != OUTPUT_LIBRARY) {
    print_error(".ROMBANKS is not yet defined.\n", ERROR_DIR);
    return FAILED;
  }

  q = input_number();

  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED || d < 0) {
    print_error(".BANK number must be zero or positive.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_rombanks <= d && g_output_format != OUTPUT_LIBRARY) {
    snprintf(g_error_message, sizeof(g_error_message), "ROM banks == %d, selected bank %d.\n", g_rombanks, d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  g_bank = d;
  g_bank_defined = 1;

  if (compare_next_token("SLOT") == SUCCEEDED) {
    skip_next_token();

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q == INPUT_NUMBER_STRING || q == INPUT_NUMBER_ADDRESS_LABEL) {
      /* turn the label into a number */
      if (_get_slot_number_by_its_name(g_label, &d) == FAILED)
        return FAILED;
      q = SUCCEEDED;
    }
    else if (q == SUCCEEDED) {
      /* is the number a direct SLOT number, or is it an address? */
      q = _get_slot_number_by_a_value(d, &d);
    }
    if (q != SUCCEEDED) {
      print_error("Cannot find the SLOT.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_slots[d].size == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "There is no SLOT number %d.\n", d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (g_output_format != OUTPUT_LIBRARY)
      fprintf(g_file_out_ptr, "B%d %d ", g_bank, d);

    ind = g_bank;
    inz = d;
    g_current_slot = d;
  }
  else if (g_output_format != OUTPUT_LIBRARY) {
    fprintf(g_file_out_ptr, "B%d %d ", d, g_defaultslot);
    ind = d;
    inz = g_defaultslot;
    g_current_slot = g_defaultslot;
  }

  if (g_slots[inz].size < g_banks[ind]) {
    snprintf(g_error_message, sizeof(g_error_message), "SLOT %d's size %d < BANK %d's size %d.\n", inz, g_slots[inz].size, ind, g_banks[ind]);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }
  if (g_slots[inz].size > g_banks[ind]) {
    snprintf(g_error_message, sizeof(g_error_message), "SLOT %d's size %d > BANK %d's size %d, but the bank fits inside.\n", inz, g_slots[inz].size, ind, g_banks[ind]);
    print_error(g_error_message, ERROR_WRN);
  }

  return SUCCEEDED;
}


int directive_dbm_dwm_dlm(void) {
  
  struct macro_static *m;
  char bak[MAX_NAME_LENGTH + 1];
  
  strcpy(bak, cp);
  inz = input_number();
  if (inz != INPUT_NUMBER_ADDRESS_LABEL) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s requires macro name.\n", bak);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* find the macro */
  if (macro_get(g_label, YES, &m) == FAILED)
    return FAILED;
  if (m == NULL) {
    if (macro_get(g_label, NO, &m) == FAILED)
      return FAILED;
  }

  if (m == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "No MACRO \"%s\" defined.\n", g_label);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (strcaselesscmp(cp, "DBM") == 0) {
    if (macro_start_dxm(m, MACRO_CALLER_DBM, cp, YES) == FAILED)
      return FAILED;
  }
  else if (strcaselesscmp(cp, "DLM") == 0) {
    if (macro_start_dxm(m, MACRO_CALLER_DLM, cp, YES) == FAILED)
      return FAILED;
  }
  else {
    if (macro_start_dxm(m, MACRO_CALLER_DWM, cp, YES) == FAILED)
      return FAILED;
  }

  return SUCCEEDED;
}


int directive_table(void) {

  char bak[256];

  inz = input_number();
  for (table_size = 0; table_size < (int)sizeof(table_format) && (inz == INPUT_NUMBER_STRING || inz == INPUT_NUMBER_ADDRESS_LABEL); ) {
    if (strcaselesscmp(g_label, "db") == 0 || strcaselesscmp(g_label, "byte") == 0 || strcaselesscmp(g_label, "byt") == 0) {
      table_format[table_size++] = 'b';
    }
    else if (strcaselesscmp(g_label, "ds") == 0 || strcaselesscmp(g_label, "dsb") == 0) {
      strcpy(bak, g_label);

      inz = input_number();
      if (inz == FAILED)
        return FAILED;
      if (inz != SUCCEEDED) {
        snprintf(g_error_message, sizeof(g_error_message), "%s needs size.\n", bak);
        print_error(g_error_message, ERROR_INP);
        return FAILED;
      }

      for (inz = 0; inz < d && table_size < (int)sizeof(table_format); inz++)
        table_format[table_size++] = 'b';
    }
    else if (strcaselesscmp(g_label, "dw") == 0 || strcaselesscmp(g_label, "word") == 0 || strcaselesscmp(g_label, "addr") == 0) {
      table_format[table_size++] = 'w';
    }
    else if (strcaselesscmp(g_label, "dsw") == 0) {
      strcpy(bak, g_label);

      inz = input_number();
      if (inz == FAILED)
        return FAILED;
      if (inz != SUCCEEDED) {
        snprintf(g_error_message, sizeof(g_error_message), "%s needs size.\n", bak);
        print_error(g_error_message, ERROR_INP);
        return FAILED;
      }

      for (inz = 0; inz < d && table_size < (int)sizeof(table_format); inz++)
        table_format[table_size++] = 'w';
    }
#ifdef W65816
    else if (strcaselesscmp(g_label, "dl") == 0 || strcaselesscmp(g_label, "long") == 0 || strcaselesscmp(g_label, "faraddr") == 0) {
      table_format[table_size++] = 'l';
    }
    else if (strcaselesscmp(g_label, "dsl") == 0) {
      strcpy(bak, g_label);

      inz = input_number();
      if (inz == FAILED)
        return FAILED;
      if (inz != SUCCEEDED) {
        snprintf(g_error_message, sizeof(g_error_message), "%s needs size.\n", bak);
        print_error(g_error_message, ERROR_INP);
        return FAILED;
      }

      for (inz = 0; inz < d && table_size < (int)sizeof(table_format); inz++)
        table_format[table_size++] = 'l';
    }
#endif
    else {
      snprintf(g_error_message, sizeof(g_error_message), "Unknown symbol \"%s\".\n", g_label);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
      
    inz = input_number();
  }

  if (table_size >= (int)sizeof(table_format)) {
    snprintf(g_error_message, sizeof(g_error_message), ".TABLE is out of size.\n");
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (inz == FAILED)
    return FAILED;
  else if (inz == INPUT_NUMBER_EOL && table_size == 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".TABLE needs data.\n");
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }
  else if (inz == INPUT_NUMBER_EOL)
    next_line();
  else {
    snprintf(g_error_message, sizeof(g_error_message), "Unknown symbol.\n");
    print_error(g_error_message, ERROR_DIR);
    return FAILED;      
  }

  table_defined = 1;
  table_index = 0;

  return SUCCEEDED;    
}


int directive_row_data(void) {

  char bak[256];
  int rows = 0;
  
  strcpy(bak, cp);

  if (table_defined == 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".TABLE needs to be defined before .%s can be used.\n", bak);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }
    
  if (strcaselesscmp(bak, "ROW") == 0) {
    if (table_index != 0) {
      snprintf(g_error_message, sizeof(g_error_message), ".ROW cannot be used here. .DATA needs to be used again to give the remaining of the row.\n");
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
  }

  inz = input_number();
  ind = 0;
  for ( ; inz == SUCCEEDED || inz == INPUT_NUMBER_STRING || inz == INPUT_NUMBER_ADDRESS_LABEL || inz == INPUT_NUMBER_STACK; ) {
    if (inz == INPUT_NUMBER_STRING) {
      if (table_format[table_index] == 'b') {
        if (strlen(g_label) != 1) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s was expecting a byte, got %d bytes instead.\n", bak, (int)strlen(g_label));
          print_error(g_error_message, ERROR_INP);
          return FAILED;
        }

        fprintf(g_file_out_ptr, "d%d ", g_label[0]);          
      }
      else if (table_format[table_index] == 'w') {
        if (strlen(g_label) > 2 || strlen(g_label) <= 0) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s was expecting a word (2 bytes), got %d bytes instead.\n", bak, (int)strlen(g_label));
          print_error(g_error_message, ERROR_INP);
          return FAILED;
        }

        fprintf(g_file_out_ptr, "y%d ", (g_label[0] << 8) | g_label[1]);
      }
#ifdef W65816
      else if (table_format[table_index] == 'l') {
        if (strlen(g_label) > 3 || strlen(g_label) <= 0) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s was expecting a long (3 bytes), got %d bytes instead.\n", bak, (int)strlen(g_label));
          print_error(g_error_message, ERROR_INP);
          return FAILED;
        }

        fprintf(g_file_out_ptr, "z%d ", (g_label[0] << 16) | (g_label[1] << 8) | g_label[2]);
      }
#endif
      else {
        snprintf(g_error_message, sizeof(g_error_message), ".%s has encountered an unsupported internal datatype \"%c\".\n", bak, table_format[table_index]);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
    }
    else if (inz == SUCCEEDED) {
      if (table_format[table_index] == 'b') {
        if (d < -128 || d > 255) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s expects 8-bit data, %d is out of range!\n", bak, d);
          print_error(g_error_message, ERROR_DIR);
          return FAILED;
        }
    
        fprintf(g_file_out_ptr, "d%d ", d);
      }
      else if (table_format[table_index] == 'w') {
        if (d < -32768 || d > 65535) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s expects 16-bit data, %d is out of range!\n", bak, d);
          print_error(g_error_message, ERROR_DIR);
          return FAILED;
        }

        fprintf(g_file_out_ptr, "y%d ", d);
      }
#ifdef W65816
      else if (table_format[table_index] == 'l') {
        if (d < -8388608 || d > 16777215) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s expects 24-bit data, %d is out of range!\n", bak, d);
          print_error(g_error_message, ERROR_DIR);
          return FAILED;
        }

        fprintf(g_file_out_ptr, "z%d ", d);
      }
#endif
      else {
        snprintf(g_error_message, sizeof(g_error_message), ".%s has encountered an unsupported internal datatype \"%c\".\n", bak, table_format[table_index]);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
    }
    else if (inz == INPUT_NUMBER_ADDRESS_LABEL) {
      if (table_format[table_index] == 'b') {
        fprintf(g_file_out_ptr, "k%d Q%s ", g_active_file_info_last->line_current, g_label);
      }
      else if (table_format[table_index] == 'w') {
        fprintf(g_file_out_ptr, "k%d r%s ", g_active_file_info_last->line_current, g_label);
      }
#ifdef W65816
      else if (table_format[table_index] == 'l') {
        fprintf(g_file_out_ptr, "k%d q%s ", g_active_file_info_last->line_current, g_label);
      }
#endif
      else {
        snprintf(g_error_message, sizeof(g_error_message), ".%s has encountered an unsupported internal datatype \"%c\".\n", bak, table_format[table_index]);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
    }
    else if (inz == INPUT_NUMBER_STACK) {
      if (table_format[table_index] == 'b') {
        fprintf(g_file_out_ptr, "c%d ", g_latest_stack);
      }
      else if (table_format[table_index] == 'w') {
        fprintf(g_file_out_ptr, "C%d ", g_latest_stack);
      }
#ifdef W65816
      else if (table_format[table_index] == 'l') {
        fprintf(g_file_out_ptr, "T%d ", g_latest_stack);
      }
#endif
      else {
        snprintf(g_error_message, sizeof(g_error_message), ".%s has encountered an unsupported internal datatype \"%c\".\n", bak, table_format[table_index]);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
    }

    ind++;
    table_index++;
    if (table_index >= table_size) {
      rows++;
      table_index = 0;
    }

    inz = input_number();
  }

  if (inz == FAILED)
    return FAILED;

  if (inz == INPUT_NUMBER_EOL && ind == 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs data.\n", bak);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (strcaselesscmp(bak, "ROW") == 0) {
    if (table_index != 0 || rows != 1) {
      snprintf(g_error_message, sizeof(g_error_message), ".ROW needs exactly one row of data, no more, no less.\n");
      print_error(g_error_message, ERROR_INP);
      return FAILED;
    }
  }

  if (inz == INPUT_NUMBER_EOL)
    next_line();

  return SUCCEEDED;
}


int directive_db_byt_byte(void) {

  char bak[256];
  int o;

  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  strcpy(bak, cp);

  inz = input_number();
  for (ind = 0; inz == SUCCEEDED || inz == INPUT_NUMBER_STRING || inz == INPUT_NUMBER_ADDRESS_LABEL || inz == INPUT_NUMBER_STACK; ind++) {
    if (inz == INPUT_NUMBER_STRING) {
      for (o = 0; o < g_string_size; o++) {
        /* handle '\0' */
        if (g_label[o] == '\\' && g_label[o + 1] == '0') {
          fprintf(g_file_out_ptr, "d%d ", '\0');
          o++;
        }
        /* handle '\x' */
        else if (g_label[o] == '\\' && g_label[o + 1] == 'x') {
          char tmp_a[8], *tmp_b;
          int tmp_c;
        
          o += 3;
          snprintf(tmp_a, sizeof(tmp_a), "%c%c", g_label[o-1], g_label[o]);
          tmp_c = (int)strtol(tmp_a, &tmp_b, 16);
          if (*tmp_b) {
            snprintf(g_error_message, sizeof(g_error_message), ".%s '\\x' needs hexadecimal byte (00-FF) data.\n", bak);
            print_error(g_error_message, ERROR_INP);
            return FAILED;
          }
          fprintf(g_file_out_ptr, "d%d ", tmp_c);
        }
        /* handle '\<' */
        else if (g_label[o] == '\\' && g_label[o + 1] == '<') {
          o += 2;
          if (o == g_string_size) {
            snprintf(g_error_message, sizeof(g_error_message), ".%s '\\<' needs character data.\n", bak);
            print_error(g_error_message, ERROR_INP);
            return FAILED;
          }
          fprintf(g_file_out_ptr, "d%d ", (int)g_label[o]|0x80);
        }
        /* handle '\>' */
        else if (g_label[o] == '\\' && g_label[o + 1] == '>' && o == 0) {
          snprintf(g_error_message, sizeof(g_error_message), ".%s '\\>' needs character data.\n", bak);
          print_error(g_error_message, ERROR_INP);
          return FAILED;
        }
        else if (g_label[o + 1] == '\\' && g_label[o + 2] == '>') {
          fprintf(g_file_out_ptr, "d%d ", (int)g_label[o]|0x80);
          o += 2;
        }
        /* handle '\\' */
        else if (g_label[o] == '\\' && g_label[o + 1] == '\\') {
          fprintf(g_file_out_ptr, "d%d ", '\\');
          o++;
        }
        else
          fprintf(g_file_out_ptr, "d%d ", (int)g_label[o]);
      }
      inz = input_number();
      continue;
    }

    if (inz == SUCCEEDED && (d < -128 || d > 255)) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s expects 8-bit data, %d is out of range!\n", bak, d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (inz == SUCCEEDED)
      fprintf(g_file_out_ptr, "d%d ", d);
    else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
      fprintf(g_file_out_ptr, "Q%s ", g_label);
    else if (inz == INPUT_NUMBER_STACK)
      fprintf(g_file_out_ptr, "c%d ", g_latest_stack);

    inz = input_number();
  }

  if (inz == FAILED)
    return FAILED;

  if (inz == INPUT_NUMBER_EOL && ind == 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs data.\n", bak);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (inz == INPUT_NUMBER_EOL)
    next_line();

  return SUCCEEDED;
}


int directive_asctable_asciitable(void) {
  
  int astart, aend, q, o;
  char bak[256];
  
  strcpy(bak, cp);

  /* clear the table (to the default n->n -mapping) */
  for (o = 0; o < 256; o++)
    g_asciitable[o] = o;

  /* read the entries */
  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDA") == 0)
      break;
    else if (strcaselesscmp(tmp, "MAP") == 0) {
      q = input_number();

      while (q == INPUT_NUMBER_EOL) {
        next_line();
        q = input_number();
      }

      if (q == FAILED)
        return FAILED;
      if (q == SUCCEEDED && (d < 0 || d > 255)) {
        print_error("The entry must be a positive 8-bit immediate value or one letter string.\n", ERROR_DIR);
        return FAILED;
      }
      if (q == INPUT_NUMBER_STRING) {
        if (g_string_size != 1) {
          print_error("The entry must be a positive 8-bit immediate value or one letter string.\n", ERROR_DIR);
          return FAILED;
        }
        else {
          d = g_label[0];
          if (d < 0)
            d += 256;
        }
      }

      astart = d;
      aend = d+1;

      /* do we have a range? */
      if (compare_next_token("TO") == SUCCEEDED) {
        skip_next_token();

        q = input_number();

        if (q == FAILED)
          return FAILED;
        if (q == SUCCEEDED && (d < 0 || d > 255)) {
          print_error("The entry must be a positive 8-bit immediate value or one letter string.\n", ERROR_DIR);
          return FAILED;
        }
        if (q == INPUT_NUMBER_STRING) {
          if (g_string_size != 1) {
            print_error("The entry must be a positive 8-bit immediate value or one letter string.\n", ERROR_DIR);
            return FAILED;
          }
          else {
            d = g_label[0];
            if (d < 0)
              d += 256;
          }
        }

        aend = d+1;
      }

      if (aend <= astart) {
        print_error("The end address of the mapping must be larger than the staring address.\n", ERROR_DIR);
        return FAILED;
      }

      /* skip the "=" */
      if (compare_next_token("=") != SUCCEEDED) {
        ind = FAILED;
        break;
      }
      skip_next_token();

      /* read the starting address */
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q == SUCCEEDED && (d < 0 || d > 255)) {
        print_error("The entry must be a positive 8-bit immediate value or one letter string.\n", ERROR_DIR);
        return FAILED;
      }
      if (q != SUCCEEDED) {
        print_error("The entry must be a positive 8-bit immediate value.\n", ERROR_DIR);
        return FAILED;
      }

      /* build the mapping */
      for (o = astart; o < aend; o++) {
        if (d >= 256) {
          print_error("The mapping overflows from the ASCII table!\n", ERROR_DIR);
          return FAILED;
        }
        g_asciitable[o] = d++;
      }
    }
    else {
      ind = FAILED;
      break;
    }
  }

  if (ind != SUCCEEDED) {
    snprintf(g_error_message, sizeof(g_error_message), "Error in .%s data structure.\n", bak);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  g_asciitable_defined = 1;

  return SUCCEEDED;
}


int directive_asc(void) {

  char bak[256];
  int o, q;

  strcpy(bak, cp);

  if (g_asciitable_defined == 0) {
    print_error("No .ASCIITABLE defined. Using the default n->n -mapping.\n", ERROR_WRN);
    for (o = 0; o < 256; o++)
      g_asciitable[o] = o;
  }

  while (1) {
    q = input_number();
    if (q == INPUT_NUMBER_EOL) {
      next_line();
      break;
    }

    if (q == INPUT_NUMBER_STRING) {
      /* remap the ascii string */
      for (o = 0; o < g_string_size; o++) {
        ind = g_label[o];
        /* handle '\0' */
        if (g_label[o] == '\\' && g_label[o + 1] == '0') {
          ind = '\0';
          fprintf(g_file_out_ptr, "d%d ", ind);
          o++;
        }
        /* handle '\x' */
        else if (g_label[o] == '\\' && g_label[o + 1] == 'x') {
          char tmp_a[8], *tmp_b;
          int tmp_c;
      
          o += 3;
          snprintf(tmp_a, sizeof(tmp_a), "%c%c", g_label[o-1], g_label[o]);
          tmp_c = (int)strtol(tmp_a, &tmp_b, 16);
          if (*tmp_b) {
            snprintf(g_error_message, sizeof(g_error_message), ".%s '\\x' needs hexadecimal byte (00-FF) data.\n", bak);
            print_error(g_error_message, ERROR_INP);
            return FAILED;
          }
          ind = tmp_c;
          fprintf(g_file_out_ptr, "d%d ", ind);
        }
        else {
          int tmp_a = 0;
        
          /* handle '\<' */
          if (g_label[o] == '\\' && g_label[o + 1] == '<') {
            o += 2;
            if (o == g_string_size) {
              snprintf(g_error_message, sizeof(g_error_message), ".%s '\\<' needs character data.\n", bak);
              print_error(g_error_message, ERROR_INP);
              return FAILED;
            }
            tmp_a = 0x80;
            ind = g_label[o];
          }
          /* handle '\>' */
          else if (g_label[o] == '\\' && g_label[o + 1] == '>' && o == 0) {
            snprintf(g_error_message, sizeof(g_error_message), ".%s '\\>' needs character data.\n", bak);
            print_error(g_error_message, ERROR_INP);
            return FAILED;
          }
          else if (g_label[o + 1] == '\\' && g_label[o + 2] == '>') {
            tmp_a = 0x80;
            o += 2;
          }
          /* handle '\\' */
          else if (g_label[o] == '\\' && g_label[o + 1] == '\\') {
            ind = '\\';
            o++;
          }
          if (ind < 0)
            ind += 256;
          ind = (int)g_asciitable[ind];
          fprintf(g_file_out_ptr, "d%d ", ind|tmp_a);
        }
      }
    }
    else if (q == SUCCEEDED) {
      /* remap the byte */
      if (d < 0 || d > 255) {
        snprintf(g_error_message, sizeof(g_error_message), ".%s needs string / byte (0-255) data.\n", bak);
        print_error(g_error_message, ERROR_INP);
        return FAILED;
      }
      ind = (int)g_asciitable[d];
      fprintf(g_file_out_ptr, "d%d ", ind);
    }
    else {
      snprintf(g_error_message, sizeof(g_error_message), ".%s needs string / byte (0-255) data.\n", bak);
      print_error(g_error_message, ERROR_INP);
      return FAILED;
    }
  }

  return SUCCEEDED;
}


int directive_dw_word_addr(void) {

  char bak[256];

  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  strcpy(bak, cp);

  inz = input_number();
  for (ind = 0; inz == SUCCEEDED || inz == INPUT_NUMBER_ADDRESS_LABEL || inz == INPUT_NUMBER_STACK; ind++) {
    if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s expects 16-bit data, %d is out of range!\n", bak, d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (inz == SUCCEEDED)
      fprintf(g_file_out_ptr, "y%d", d);
    else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
      fprintf(g_file_out_ptr, "r%s ", g_label);
    else if (inz == INPUT_NUMBER_STACK)
      fprintf(g_file_out_ptr, "C%d ", g_latest_stack);

    inz = input_number();
  }

  if (inz == FAILED)
    return FAILED;

  if ((inz == INPUT_NUMBER_EOL || inz == INPUT_NUMBER_STRING) && ind == 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs data.\n", bak);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (inz == INPUT_NUMBER_EOL)
    next_line();

  return SUCCEEDED;
}


#ifdef W65816

int directive_dl_long_faraddr(void) {

  char bak[256];

  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  strcpy(bak, cp);

  inz = input_number();
  for (ind = 0; inz == SUCCEEDED || inz == INPUT_NUMBER_ADDRESS_LABEL || inz == INPUT_NUMBER_STACK; ind++) {
    if (inz == SUCCEEDED && (d < -8388608 || d > 16777215)) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s expects 24-bit data, %d is out of range!\n", bak, d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (inz == SUCCEEDED)
      fprintf(g_file_out_ptr, "z%d ", d);
    else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
      fprintf(g_file_out_ptr, "q%s ", g_label);
    else if (inz == INPUT_NUMBER_STACK)
      fprintf(g_file_out_ptr, "T%d ", g_latest_stack);

    inz = input_number();
  }

  if (inz == FAILED)
    return FAILED;

  if ((inz == INPUT_NUMBER_EOL || inz == INPUT_NUMBER_STRING) && ind == 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs data.\n", bak);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (inz == INPUT_NUMBER_EOL)
    next_line();

  return SUCCEEDED;
}


int directive_dsl(void) {

  int q;
  
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    print_error(".DSL needs size.\n", ERROR_INP);
    return FAILED;
  }

  if (d < 1 || d > 65535) {
    snprintf(g_error_message, sizeof(g_error_message), ".DSL expects a 16-bit positive integer as size, %d is out of range!\n", d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  inz = d;

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (!(q == SUCCEEDED || q == INPUT_NUMBER_ADDRESS_LABEL || q == INPUT_NUMBER_STACK)) {
    print_error(".DSL needs data.\n", ERROR_INP);
    return FAILED;
  }

  if (q == SUCCEEDED && (d < -8388608 || d > 16777215)) {
    snprintf(g_error_message, sizeof(g_error_message), ".DSL expects 24-bit data, %d is out of range!\n", d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED)
    fprintf(g_file_out_ptr, "h%d %d ", inz, d);
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);
    for (q = 0; q < inz; q++)
      fprintf(g_file_out_ptr, "q%s ", g_label);
  }
  else if (q == INPUT_NUMBER_STACK) {
    for (q = 0; q < inz; q++)
      fprintf(g_file_out_ptr, "T%d ", g_latest_stack);
  }

  return SUCCEEDED;
}


int directive_name_w65816(void) {
    
  no_library_files(".NAME");

  if ((ind = get_next_token()) == FAILED)
    return FAILED;

  if (ind != GET_NEXT_TOKEN_STRING) {
    print_error(".NAME requires a string of 1 to 21 letters.\n", ERROR_DIR);
    return FAILED;
  }

  /* no name has been defined so far */
  if (g_name_defined == 0) {
    for (ind = 0; tmp[ind] != 0 && ind < 21; ind++)
      g_name[ind] = tmp[ind];

    if (ind == 21 && tmp[ind] != 0) {
      print_error(".NAME requires a string of 1 to 21 letters.\n", ERROR_DIR);
      return FAILED;
    }

    for ( ; ind < 21; g_name[ind] = 0, ind++)
      ;

    g_name_defined = 1;
  }
  else {
    /* compare the names */
    for (ind = 0; tmp[ind] != 0 && g_name[ind] != 0 && ind < 21; ind++)
      if (g_name[ind] != tmp[ind])
        break;

    if (ind == 21 && tmp[ind] != 0) {
      print_error(".NAME requires a string of 1 to 21 letters.\n", ERROR_DIR);
      return FAILED;
    }
    if (ind != 21 && (g_name[ind] != 0 || tmp[ind] != 0)) {
      print_error(".NAME was already defined.\n", ERROR_DIR);
      return FAILED;
    }
  }

  return SUCCEEDED;
}

#endif


/* this is used for legacy .DSTRUCT syntax, and only for generating labels in the new
 * .DSTRUCT syntax. */
int parse_dstruct_entry(char *iname, struct structure *s, int *labels_only) {

  char tmpname[MAX_NAME_LENGTH*2+10];
  struct structure_item *it;
  int f, o, c, g;

  /* read the data */
  it = s->items;
  for (ind = 0; it != NULL; ind++) {
    snprintf(tmpname, sizeof(tmpname), "%s.%s", iname, it->name);
    if (verify_name_length(tmpname) == FAILED)
      return FAILED;

    if (it->type != STRUCTURE_ITEM_TYPE_UNION) { /* add field label */
      char full_label[MAX_NAME_LENGTH + 1];

      fprintf(g_file_out_ptr, "k%d L%s ", g_active_file_info_last->line_current, tmpname);
    
      if (get_full_label(tmpname, full_label) == FAILED)
        return FAILED;
      if (add_label_sizeof(full_label, it->size) == FAILED)
        return FAILED;
    }

    if (it->type == STRUCTURE_ITEM_TYPE_UNION) {
      if (*labels_only == NO) {
        print_error(".DSTRUCT doesn't support structs with unions.\n", ERROR_DIR);
        return FAILED;
      }
      else {
        struct structure *us;

        us = it->union_items;
        while (us != NULL) {
          if (us->name[0] != '\0') { /* check if the union is named */
            char full_label[MAX_NAME_LENGTH + 1];

            snprintf(tmpname, sizeof(tmpname), "%s.%s", iname, us->name);
            if (verify_name_length(tmpname) == FAILED)
              return FAILED;

            fprintf(g_file_out_ptr, "k%d L%s ", g_active_file_info_last->line_current, tmpname);

            if (get_full_label(tmpname, full_label) == FAILED)
              return FAILED;
            if (add_label_sizeof(full_label, us->size) == FAILED)
              return FAILED;
          }
          else
            strcpy(tmpname, iname);

          parse_dstruct_entry(tmpname, us, labels_only);
          fprintf(g_file_out_ptr, "o%d 0 ", -us->size); /* rewind */
          us = us->next;
        }

        fprintf(g_file_out_ptr, "o%d 0 ", it->size); /* jump to union end */
      }
    }
    else if (it->type == STRUCTURE_ITEM_TYPE_INSTANCEOF) {
      /* handle .INSTANCEOF directive */
      /* update the naming prefix */
      snprintf(tmpname, sizeof(tmpname), "%s.%s", iname, it->name);
      if (verify_name_length(tmpname) == FAILED)
        return FAILED;

      if (it->num_instances == 1) {
        if (parse_dstruct_entry(tmpname, it->instance, labels_only) == FAILED)
          return FAILED;
      }
      else {
        int labels_only_tmp = YES;

        snprintf(tmpname, sizeof(tmpname), "%s.%s", iname, it->name);

        /* we have "struct.instance" and "struct.1.instance" referencing the same data. */
        parse_dstruct_entry(tmpname, it->instance, &labels_only_tmp);

        /* return to start of struct */
        fprintf(g_file_out_ptr, "o%d 0 ", -it->instance->size);

        for (g = 1; g <= it->num_instances; g++) {
          snprintf(tmpname, sizeof(tmpname), "%s.%s.%d", iname, it->name, g);
          if (verify_name_length(tmpname) == FAILED)
            return FAILED;

          fprintf(g_file_out_ptr, "k%d L%s ", g_active_file_info_last->line_current, tmpname);

          if (add_label_sizeof(tmpname, it->instance->size) == FAILED)
            return FAILED;
          if (parse_dstruct_entry(tmpname, it->instance, labels_only) == FAILED)
            return FAILED;
        }
      }

      it = it->next;
      continue;
    }
    else if (it->size == 0 || it->type == STRUCTURE_ITEM_TYPE_DOTTED) {
      /* don't put data into empty structure items */
      it = it->next;
      continue;
    }
    else {
      if (*labels_only == NO) {
        /* take care of the strings */
        if (inz == INPUT_NUMBER_STRING) {
          if (it->size < g_string_size) {
            snprintf(g_error_message, sizeof(g_error_message), "String \"%s\" doesn't fit into the %d bytes of \"%s.%s\". Discarding the overflow.\n", g_label, it->size, s->name, it->name);
            print_error(g_error_message, ERROR_WRN);
            c = it->size;
          }
          else
            c = g_string_size;

          /* copy the string */
          for (o = 0; o < c; o++)
            fprintf(g_file_out_ptr, "d%d ", (int)g_label[o]);
        }
        /* take care of the rest */
        else {
          if (it->size == 1) {
            if ((inz == SUCCEEDED) && (d < -128 || d > 255)) {
              snprintf(g_error_message, sizeof(g_error_message), "\"%s.%s\" expects 8-bit data, %d is out of range!\n", s->name, it->name, d);
              print_error(g_error_message, ERROR_DIR);
              return FAILED;
            }

            if (inz == SUCCEEDED)
              fprintf(g_file_out_ptr, "d%d ", d);
            else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
              fprintf(g_file_out_ptr, "k%d Q%s ", g_active_file_info_last->line_current, g_label);
            else if (inz == INPUT_NUMBER_STACK)
              fprintf(g_file_out_ptr, "c%d ", g_latest_stack);

            o = 1;
          }
          else {
            if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
              snprintf(g_error_message, sizeof(g_error_message), "\"%s.%s\" expects 16-bit data, %d is out of range!\n", s->name, it->name, d);
              print_error(g_error_message, ERROR_DIR);
              return FAILED;
            }

            if (inz == SUCCEEDED)
              fprintf(g_file_out_ptr, "y%d", d);
            else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
              fprintf(g_file_out_ptr, "k%d r%s ", g_active_file_info_last->line_current, g_label);
            else if (inz == INPUT_NUMBER_STACK)
              fprintf(g_file_out_ptr, "C%d ", g_latest_stack);

            o = 2;
          }
          /* TODO: longs */
        }
        /* fill the rest of the item with emptyfill or zero */
        if (g_emptyfill_defined != 0)
          f = g_emptyfill;
        else
          f = 0;

        for (; o < it->size; o++)
          fprintf(g_file_out_ptr, "d%d ", f);
      }
      else { /* labels_only == YES */
        fprintf(g_file_out_ptr, "o%d 0 ", it->size);
      }
    }

    it = it->next;

    if (*labels_only == NO) {
      inz = input_number();
      if (!(inz == SUCCEEDED || inz == INPUT_NUMBER_STRING || inz == INPUT_NUMBER_ADDRESS_LABEL || inz == INPUT_NUMBER_STACK))
        *labels_only = YES; /* ran out of data to read */
    }
  }

  return SUCCEEDED;
}

/* search for "field_name" within a structure. return the corresponding structure_item and
   the offset within the structure it's located at. recurses through instanceof's and
   unions. */
int find_struct_field(struct structure *s, char *field_name, int *item_size, int *field_offset) {

  int offset = 0;
  char prefix[MAX_NAME_LENGTH + 1];
  char *after_dot;
  struct structure_item *si;

  strcpy(prefix, field_name);
  if (strchr(prefix, '.') != NULL) {
    *strchr(prefix, '.') = '\0';
    after_dot = field_name + strlen(prefix) + 1;
  }
  else
    after_dot = NULL;

  si = s->items;

  while (si != NULL) {
    if (si->type == STRUCTURE_ITEM_TYPE_UNION) {
      /* unions don't necessarily have names, so we need to check them all */
      struct structure *us;

      us = si->union_items;
      while (us != NULL) {
        if (us->name[0] != '\0') { /* has name */
          if (strcmp(field_name, us->name) == 0) {
            *item_size = us->size;
            *field_offset = offset;
            return SUCCEEDED;
          }
          if (after_dot != NULL && strcmp(prefix, us->name) == 0) {
            if (find_struct_field(us, after_dot, item_size, field_offset) == SUCCEEDED) {
              *field_offset += offset;
              return SUCCEEDED;
            }
          }
        }
        /* no name */
        else if (find_struct_field(us, field_name, item_size, field_offset) == SUCCEEDED) {
          *field_offset += offset;
          return SUCCEEDED;
        }
        us = us->next;
      }
    }
    else if (strcmp(field_name, si->name) == 0) {
      *field_offset = offset;
      *item_size = si->size;
      return SUCCEEDED;
    }
    /* look for prefix for an ".instanceof" */
    else if (after_dot != NULL && strcmp(prefix, si->name) == 0) {
      if (si->type == STRUCTURE_ITEM_TYPE_INSTANCEOF) {
        if (find_struct_field(si->instance, after_dot, item_size, field_offset) == SUCCEEDED) {
          *field_offset += offset;
          return SUCCEEDED;
        }
        /* look for ie. "struct.1.field" */
        else if (si->num_instances > 1) {
          int g;
          for (g = 1; g <= si->num_instances; g++) {
            char num_str[256];
        
            snprintf(num_str, sizeof(num_str), "%d", g);
            if (strncmp(num_str, after_dot, strlen(num_str)) == 0) {
              /* entire string matched? */
              if (strcmp(num_str, after_dot) == 0) {
                *field_offset = offset + (g-1) * si->instance->size;
                *item_size = si->instance->size;
                return SUCCEEDED;
              }
              /* only prefix matched */
              if (after_dot[strlen(num_str)] == '.' && find_struct_field(si->instance, after_dot + strlen(num_str) + 1, item_size, field_offset) == SUCCEEDED) {
                *field_offset += offset + (g-1) * si->instance->size;
                return SUCCEEDED;
              }
            }
          }
        }
      }
      /* else keep looking */
    }

    if (si->type != STRUCTURE_ITEM_TYPE_DOTTED)
      offset += si->size;

    si = si->next;
  }

  return FAILED;
}


int directive_dstruct(void) {

  char iname[MAX_NAME_LENGTH*2+5];
  struct structure *s;
  int q, q2;
  int labels_only;

  if (compare_next_token("INSTANCEOF") == SUCCEEDED) { /* nameless */
    skip_next_token();
    iname[0] = '\0';
  }
  else {
    /* get instance name */
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != INPUT_NUMBER_ADDRESS_LABEL) {
      print_error(".DSTRUCT needs a name for the instance.\n", ERROR_INP);
      return FAILED;
    }
    strcpy(iname, g_label);

    if (compare_next_token("INSTANCEOF") == SUCCEEDED)
      skip_next_token();
  }

  /* get structure name */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".DSTRUCT needs a structure name.\n", ERROR_INP);
    return FAILED;
  }

  /* find the structure */
  s = get_structure(g_label);

  if (s == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Reference to an unidentified structure \"%s\".\n", g_label);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* DEBUG
     {
     struct structure_item *sS = s->items;
        
     fprintf(stderr, "STRUCT \"%s\" size %d\n", s->name, s->size);
     while (sS != NULL) {
     fprintf(stderr, "ITEM \"%s\" size %d\n", sS->name, sS->size);
     sS = sS->next;
     }
     }
  */

  if (iname[0] != '\0') {
    char full_label[MAX_NAME_LENGTH + 1];
    
    fprintf(g_file_out_ptr, "k%d L%s ", g_active_file_info_last->line_current, iname);

    if (get_full_label(iname, full_label) == FAILED)
      return FAILED;
    if (add_label_sizeof(full_label, s->size) == FAILED)
      return FAILED;
  }

  if (compare_next_token("VALUES") == SUCCEEDED) {
    /* new syntax */

    int field_offset;
    char field_name[MAX_NAME_LENGTH + 1];
    int item_size;

    if (g_dstruct_status == ON) {
      print_error("You can't have nested .DSTRUCT's.\n", ERROR_DIR);
      return FAILED;
    }

    g_dstruct_status = ON;

    skip_next_token();

    fprintf(g_file_out_ptr, "e%d -1 ", s->size); /* mark start address of dstruct */

    q = get_next_token();

    while (q == SUCCEEDED) {
      if ((q2 = parse_if_directive()) != -1) {
        return q2;
      }
      if (strcaselesscmp(tmp, ".ENDST") == 0) {
        break;
      }
      else {
        if (tmp[strlen(tmp)-1] == ':')
          tmp[strlen(tmp)-1] = '\0';
        strcpy(field_name, tmp);

        if (find_struct_field(s, field_name, &item_size, &field_offset) == FAILED) {
          snprintf(g_error_message, sizeof(g_error_message), ".DSTRUCT: Couldn't find field \"%s\" in structure \"%s\".\n", field_name, s->name);
          print_error(g_error_message, ERROR_DIR);
          return FAILED;
        }

        fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);
        fprintf(g_file_out_ptr, "e%d %d ", field_offset, item_size);

        do {
          if ((q = get_next_token()) == FAILED) {
            print_error("Error parsing .DSTRUCT.\n", ERROR_ERR);
            return FAILED;
          }

          if (tmp[0] != '.' || strcaselesscmp(tmp, ".ENDST") == 0)
            break;

          if (parse_directive() == FAILED)
            return FAILED;
        }
        while (1);
      }
    }

    if (q != SUCCEEDED) {
      print_error("Error parsing .DSTRUCT.\n", ERROR_ERR);
      return FAILED;
    }

    /* now generate labels */
    if (iname[0] != '\0') {
      labels_only = YES;
      fprintf(g_file_out_ptr, "e%d -3 ", 0); /* back to start of struct */
      if (parse_dstruct_entry(iname, s, &labels_only) == FAILED)
        return FAILED;
    }

    fprintf(g_file_out_ptr, "e%d -3 ", 0); /* back to start of struct */
    fprintf(g_file_out_ptr, "e%d -2 ", s->size); /* mark end of .DSTRUCT */

    g_dstruct_status = OFF;

    return SUCCEEDED;
  }
  else if (compare_next_token("DATA") == SUCCEEDED)
    skip_next_token();

  /* legacy syntax */

  inz = input_number();
  labels_only = NO;
  if (parse_dstruct_entry(iname, s, &labels_only) == FAILED)
    return FAILED;

  if (inz == INPUT_NUMBER_EOL)
    next_line();
  else {
    snprintf(g_error_message, sizeof(g_error_message), "Too much data for structure \"%s\".\n", s->name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  return SUCCEEDED;
}


int directive_dsb_ds(void) {

  char bak[256];
  int q;
  
  strcpy(bak, cp);

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs size.\n", bak);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (d < 1 || d > 65535) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s expects a 16-bit positive integer as size, %d is out of range!\n", bak, d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  inz = d;

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (!(q == SUCCEEDED || q == INPUT_NUMBER_ADDRESS_LABEL || q == INPUT_NUMBER_STACK)) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs data.\n", bak);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (q == SUCCEEDED && (d > 255 || d < -128)) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s expects 8-bit data, %d is out of range!\n", bak, d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED)
    fprintf(g_file_out_ptr, "x%d %d ", inz, d);
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);
    for (q = 0; q < inz; q++)
      fprintf(g_file_out_ptr, "R%s ", g_label);
  }
  else if (q == INPUT_NUMBER_STACK) {
    for (q = 0; q < inz; q++)
      fprintf(g_file_out_ptr, "c%d ", g_latest_stack);
  }

  return SUCCEEDED;
}


int directive_dsw(void) {

  int q;

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    print_error(".DSW needs size.\n", ERROR_INP);
    return FAILED;
  }

  if (d < 1 || d > 65535) {
    snprintf(g_error_message, sizeof(g_error_message), ".DSW expects a 16-bit positive integer as size, %d is out of range!\n", d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  inz = d;

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (!(q == SUCCEEDED || q == INPUT_NUMBER_ADDRESS_LABEL || q == INPUT_NUMBER_STACK)) {
    print_error(".DSW needs data.\n", ERROR_INP);
    return FAILED;
  }

  if (q == SUCCEEDED && (d < -32768 || d > 65535)) {
    snprintf(g_error_message, sizeof(g_error_message), ".DSW expects 16-bit data, %d is out of range!\n", d);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED)
    fprintf(g_file_out_ptr, "X%d %d ", inz, d);
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);
    for (q = 0; q < inz; q++)
      fprintf(g_file_out_ptr, "r%s ", g_label);
  }
  else if (q == INPUT_NUMBER_STACK) {
    for (q = 0; q < inz; q++)
      fprintf(g_file_out_ptr, "C%d ", g_latest_stack);
  }

  return SUCCEEDED;
}


int directive_incdir(void) {
  
  int q, o;
  char *c;

  g_expect_calculations = NO;
  o = input_number();
  g_expect_calculations = YES;

  if (o != INPUT_NUMBER_STRING && o != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".INCDIR needs a directory string.\n", ERROR_DIR);
    return FAILED;
  }

  q = (int)strlen(g_label);

  /* use the default dir? */
  if (q == 0) {
    if (g_include_dir != NULL)
      g_include_dir[0] = 0;
    return SUCCEEDED;
  }

  /* use the given dir */
  o = (int)(strlen(g_label) + 2);
  if (o > g_include_dir_size) {
    c = realloc(g_include_dir, o);
    if (c == NULL) {
      print_error("Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }
    g_include_dir = c;
    g_include_dir_size = o;
  }

  /* convert the path string to local enviroment */
  strcpy(g_include_dir, g_label);
  localize_path(g_include_dir);

  /* terminate the string with '/' */
#ifdef MSDOS
  if (g_include_dir[q - 1] != '\\') {
    g_include_dir[q] = '\\';
    g_include_dir[q + 1] = 0;
  }
#else
  if (g_include_dir[q - 1] != '/') {
    g_include_dir[q] = '/';
    g_include_dir[q + 1] = 0;
  }
#endif

  return SUCCEEDED;
}


int directive_include(int is_real) {

  int o, include_size = 0, accumulated_name_length = 0, character_c_position = 0, got_once = NO;
  char namespace[MAX_NAME_LENGTH + 1], path[MAX_NAME_LENGTH + 1], accumulated_name[MAX_NAME_LENGTH + 1];

  if (is_real == YES) {
    /* turn the .INCLUDE/.INC into .INDLUDE/.IND to mark it as used, if ONCE is used,
       for repetitive macro calls that contain .INCLUDE/.INC... */
    o = g_source_pointer;
    while (o >= 0) {
      if (toupper(g_buffer[o+0]) == 'I' &&
          toupper(g_buffer[o+1]) == 'N' &&
          toupper(g_buffer[o+2]) == 'C') {
        character_c_position = o+2;
        break;
      }
      o--;
    }
  }

  accumulated_name[0] = 0;

  while (1) {
    if (compare_next_token("NAMESPACE") == SUCCEEDED || compare_next_token("ONCE") == SUCCEEDED)
      break;

    g_expect_calculations = NO;
    o = input_number();
    g_expect_calculations = YES;
    
    if (o == INPUT_NUMBER_EOL) {
      next_line();
      break;
    }
    else if (o != INPUT_NUMBER_STRING && o != INPUT_NUMBER_ADDRESS_LABEL) {
      print_error(".INCLUDE needs a file name string.\n", ERROR_DIR);
      return FAILED;
    }

    if (accumulated_name_length + strlen(g_label) >= sizeof(accumulated_name)) {
      print_error("The accumulated file name length >= MAX_NAME_LENGTH. Increase its size in shared.h and recompile WLA.\n", ERROR_DIR);
      return FAILED;
    }

    strcpy(&accumulated_name[accumulated_name_length], g_label);
    accumulated_name_length = (int)strlen(accumulated_name);
  }

  strcpy(path, accumulated_name);

  /* convert the path to local enviroment */
  localize_path(g_label);
  
  if (compare_next_token("NAMESPACE") != SUCCEEDED)
    namespace[0] = 0;
  else {
    skip_next_token();

    g_expect_calculations = NO;
    o = input_number();
    g_expect_calculations = YES;
    
    if (o != INPUT_NUMBER_STRING && o != INPUT_NUMBER_ADDRESS_LABEL) {
      print_error("Namespace string is missing.\n", ERROR_DIR);
      return FAILED;
    }

    strcpy(namespace, g_label);
  }

  if (compare_next_token("ONCE") == SUCCEEDED) {
    skip_next_token();

    got_once = YES;
  }
  
  if (is_real == YES) {
    if (include_file(path, &include_size, namespace) == FAILED)
      return FAILED;
  
    /* WARNING: this is tricky: did we just include a file inside a macro? */
    if (g_macro_active != 0) {
      /* yes. note that the now we added new bytes after g_source_pointer, so if a macro_return_i is
         bigger than g_source_pointer, we'll need to add the bytes to it */
      struct macro_static *ms;
      int q, w;

      for (q = 0; q < g_macro_active; q++) {
        if (g_macro_stack[q].macro_return_i > g_source_pointer)
          g_macro_stack[q].macro_return_i += include_size;
        for (w = 0; w < g_macro_stack[q].supplied_arguments; w++) {
          if (g_macro_stack[q].argument_data[w]->start > g_source_pointer)
            g_macro_stack[q].argument_data[w]->start += include_size;
        }
      }

      /* also macro starting points that are after this position need to move forward
         in memory... */
      ms = g_macros_first;
      while (ms != NULL) {
        if (ms->start > g_source_pointer)
          ms->start += include_size;
        ms = ms->next;
      }
    }

    if (got_once == YES) {
      /* turn the .INCLUDE/.INC into .INDLUDE/.IND to mark it as used, as we got ONCE,
         for repetitive macro calls that contain .INCLUDE/.INC... */
      g_buffer[character_c_position] = 'd';
    }
  }
  
  return SUCCEEDED;
}


int directive_incbin(void) {

  struct macro_static *m;
  int s, r, j, o;

  if (g_org_defined == 0 && g_output_format != OUTPUT_LIBRARY) {
    print_error("Before you can .INCBIN data you'll need to use ORG.\n", ERROR_LOG);
    return FAILED;
  }
  
  g_expect_calculations = NO;
  o = input_number();
  g_expect_calculations = YES;

  if (o != INPUT_NUMBER_STRING && o != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".INCBIN needs a file name string.\n", ERROR_DIR);
    return FAILED;
  }

  /* convert the path string to local enviroment */
  localize_path(g_label);

  if (incbin_file(g_label, &ind, &inz, &s, &r, &m) == FAILED)
    return FAILED;
  
  if (m == NULL) {
    /* D [id] [swap] [skip] [size] */
    fprintf(g_file_out_ptr, "D%d %d %d %d ", ind, inz, s, r);
  }
  else {
    /* we want to filter the data */
    struct incbin_file_data *ifd;
    struct macro_incbin *min;

    min = calloc(sizeof(struct macro_incbin), 1);
    if (min == NULL) {
      print_error("Out of memory error while starting to filter the .INCBINed data.\n", ERROR_NONE);
      return FAILED;
    }

    ifd = g_incbin_file_data_first;
    for (j = 0; j != ind; j++)
      ifd = ifd->next;

    min->data = (unsigned char *)ifd->data;
    min->swap = inz;
    min->position = s;
    min->left = r;

    if (macro_start_incbin(m, min, YES) == FAILED)
      return FAILED;
  }

  return SUCCEEDED;
}


int directive_struct(void) {

  if (g_dstruct_status == ON) {
    print_error("You can't use .STRUCT inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  active_struct = calloc(sizeof(struct structure), 1);
  if (active_struct == NULL) {
    print_error("Out of memory while allocating a new STRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  if (get_next_token() == FAILED)
    return FAILED;

  strcpy(active_struct->name, tmp);

  active_struct->items = NULL;
  active_struct->last_item = NULL;
  union_stack = NULL;

  enum_offset = 0;
  last_enum_offset = 0;
  max_enum_offset = 0;
  base_enum_offset = 0;
  enum_ord = 1;
  enum_exp = 0;
  in_struct = YES;

  return SUCCEEDED;
}


int directive_ramsection(void) {

  int q;
      
  if (g_section_status == ON) {
    snprintf(g_error_message, sizeof(g_error_message), "There is already an open section called \"%s\".\n", g_sections_last->name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }
  if (g_dstruct_status == ON) {
    print_error("You can't use .RAMSECTION inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  if (get_next_token() == FAILED)
    return FAILED;

  g_sec_tmp = calloc(sizeof(struct section_def), 1);
  if (g_sec_tmp == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Out of memory while allocating room for a new RAMSECTION \"%s\".\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  g_sec_tmp->priority = 0;
  g_sec_tmp->listfile_items = 0;
  g_sec_tmp->listfile_ints = NULL;
  g_sec_tmp->listfile_cmds = NULL;
  g_sec_tmp->maxsize_status = OFF;
  g_sec_tmp->status = SECTION_STATUS_RAM_FREE;
  g_sec_tmp->alive = ON;
  g_sec_tmp->keep = NO;
  g_sec_tmp->data = NULL;
  g_sec_tmp->filename_id = g_active_file_info_last->filename_id;
  g_sec_tmp->id = g_section_id;
  g_sec_tmp->alignment = 1;
  g_sec_tmp->offset = 0;
  g_sec_tmp->advance_org = YES;
  g_sec_tmp->nspace = NULL;
  g_section_id++;

  /* add the namespace to the ramsection's name? */
  if (g_active_file_info_last->namespace[0] != 0) {
    if (add_namespace_to_string(tmp, sizeof(tmp), "RAMSECTION") == FAILED) {
      free(g_sec_tmp);
      return FAILED;
    }
  }

  strcpy(g_sec_tmp->name, tmp);
  g_sec_tmp->next = NULL;

  /* look for duplicate sections */
  g_sec_next = g_sections_first;
  while (g_sec_next != NULL) {
    if (strcmp(g_sec_next->name, tmp) == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "RAMSECTION \"%s\" was defined for the second time.\n", tmp);
      print_error(g_error_message, ERROR_DIR);
      free(g_sec_tmp);
      return FAILED;
    }
    g_sec_next = g_sec_next->next;
  }

  g_sec_tmp->label_map = hashmap_new();

  if (g_sections_first == NULL) {
    g_sections_first = g_sec_tmp;
    g_sections_last = g_sec_tmp;
  }
  else {
    g_sections_last->next = g_sec_tmp;
    g_sections_last = g_sec_tmp;
  }

  /* check for optional BANK */
  if (compare_next_token("BANK") != SUCCEEDED)
    g_sec_tmp->bank = 0;
  else {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error(".RAMSECTION cannot take BANK when inside a library.\n", ERROR_DIR);
      return FAILED;
    }

    skip_next_token();

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || d < 0) {
      print_error("BANK number must be zero or positive.\n", ERROR_DIR);
      return FAILED;
    }

    if (d > 255 && g_output_format != OUTPUT_LIBRARY) {
      snprintf(g_error_message, sizeof(g_error_message), "RAM banks == 256 (0-255), selected bank %d.\n", d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->bank = d;
  }

  if (compare_next_token("SLOT") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error(".RAMSECTION cannot take SLOT when inside a library.\n", ERROR_DIR);
      return FAILED;
    }

    skip_next_token();

    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q == INPUT_NUMBER_STRING || q == INPUT_NUMBER_ADDRESS_LABEL) {
      /* turn the label into a number */
      if (_get_slot_number_by_its_name(g_label, &d) == FAILED)
        return FAILED;
      q = SUCCEEDED;
    }
    else if (q == SUCCEEDED) {
      /* is the number a direct SLOT number, or is it an address? */
      q = _get_slot_number_by_a_value(d, &d);
    }
    if (q != SUCCEEDED) {
      print_error("Cannot find the SLOT.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_slots[d].size == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "There is no SLOT number %d.\n", d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->slot = d;
  }

  if (compare_next_token("ORGA") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error(".RAMSECTION cannot take ORGA when inside a library.\n", ERROR_DIR);
      return FAILED;
    }

    skip_next_token();

    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error("Cannot get the ORGA.\n", ERROR_DIR);
      return FAILED;
    }

    ind = g_slots[g_sec_tmp->slot].address;
    if (d < ind || d >= (ind + g_slots[g_sec_tmp->slot].size)) {
      print_error("ORGA is outside the current SLOT.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->address = d - ind;
  }
  else if (compare_next_token("ORG") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error(".RAMSECTION cannot take ORG when inside a library.\n", ERROR_DIR);
      return FAILED;
    }

    skip_next_token();

    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error("Cannot get the ORG.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->address = d;
  }
  else
    g_sec_tmp->address = -1;
  
  fprintf(g_file_out_ptr, "S%d ", g_sec_tmp->id);

  /* align the ramsection? */
  if (compare_next_token("ALIGN") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error(".RAMSECTION cannot take ALIGN when inside a library.\n", ERROR_DIR);
      return FAILED;
    }

    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the .RAMSECTION alignment.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->alignment = d;
  }

  /* offset the ramsection? */
  if (compare_next_token("OFFSET") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error(".RAMSECTION cannot take OFFSET when inside a library.\n", ERROR_DIR);
      return FAILED;
    }

    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the .RAMSECTION offset.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->offset = d;
  }  

  /* the type of the section */
  if (compare_next_token("FORCE") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error("Libraries don't take FORCE sections.\n", ERROR_DIR);
      return FAILED;
    }
    g_sec_tmp->status = SECTION_STATUS_RAM_FORCE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("FREE") == SUCCEEDED) {
    g_sec_tmp->status = SECTION_STATUS_RAM_FREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("SEMIFREE") == SUCCEEDED) {
    g_sec_tmp->status = SECTION_STATUS_RAM_SEMIFREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("SEMISUBFREE") == SUCCEEDED) {
    g_sec_tmp->status = SECTION_STATUS_RAM_SEMISUBFREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  
  /* return the org after the section? */
  if (compare_next_token("RETURNORG") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    g_sec_tmp->advance_org = NO;
  }

  if (compare_next_token("APPENDTO") == SUCCEEDED) {
    struct append_section *append_tmp;
    
    if (skip_next_token() == FAILED)
      return FAILED;
    
    append_tmp = calloc(sizeof(struct append_section), 1);
    if (append_tmp == NULL) {
      snprintf(g_error_message, sizeof(g_error_message), "Out of memory while allocating room for a new APPENDTO \"%s\".\n", tmp);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
      
    /* get the target section name */
    if (get_next_token() == FAILED) {
      free(append_tmp);
      return FAILED;
    }

    /* add the namespace to the section's name? */
    if (strlen(tmp) > 2 && tmp[0] == '*' && tmp[1] == ':') {
      char buf[MAX_NAME_LENGTH + 1];
      
      /* nope, this goes to global namespace. now '*:' has done its job, let's remove it */
      if (strlen(tmp) >= sizeof(buf)) {
        snprintf(g_error_message, sizeof(g_error_message), "The APPENDTO string \"%s\" is too long. Increase MAX_NAME_LENGTH in shared.h and recompile WLA.\n", tmp);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      strcpy(buf, &tmp[2]);
      strcpy(tmp, buf);
    }
    else if (g_active_file_info_last->namespace[0] != 0) {
      if (add_namespace_to_string(tmp, sizeof(tmp), "APPENDTO") == FAILED)
        return FAILED;
    }
    
    strcpy(append_tmp->section, g_sec_tmp->name);
    strcpy(append_tmp->append_to, tmp);

    append_tmp->next = g_append_sections;
    g_append_sections = append_tmp;
  }

  if (compare_next_token("PRIORITY") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the .RAMSECTION priority.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->priority = d;
  }

  if (compare_next_token("KEEP") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    g_sec_tmp->keep = YES;
  }

  enum_offset = 0;
  last_enum_offset = 0;
  max_enum_offset = 0;
  base_enum_offset = 0;
  enum_ord = 1;

  /* setup active_struct (ramsection vars stored here temporarily) */
  active_struct = calloc(sizeof(struct structure), 1);
  if (active_struct == NULL) {
    print_error("Out of memory while parsing .RAMSECTION.\n", ERROR_DIR);
    return FAILED;
  }
  active_struct->name[0] = '\0';
  active_struct->items = NULL;
  active_struct->last_item = NULL;
  union_stack = NULL;
  
  in_ramsection = YES;

  /* generate a section start label? */
  if (g_extra_definitions == ON)
    generate_label("SECTIONSTART_", g_sec_tmp->name);

  return SUCCEEDED;
}


int directive_section(void) {
  
  int l;

  if (g_dstruct_status == ON) {
    print_error("You can't set the section inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }
  else if (g_section_status == ON) {
    snprintf(g_error_message, sizeof(g_error_message), "There is already an open section called \"%s\".\n", g_sections_last->name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }
  else if (g_output_format != OUTPUT_LIBRARY && g_bank_defined == 0) {
    print_error(".SECTION requires a predefined bank.\n", ERROR_DIR);
    return FAILED;
  }
  else if (g_output_format != OUTPUT_LIBRARY && g_org_defined == 0) {
    print_error(".SECTION requires a starting address for positioning.\n", ERROR_DIR);
    return FAILED;
  }

  if (get_next_token() == FAILED)
    return FAILED;

  /* every library section starts @ the beginning of the bank */
  if (g_output_format == OUTPUT_LIBRARY)
    g_org_defined = 1;

  g_sec_tmp = calloc(sizeof(struct section_def), 1);
  if (g_sec_tmp == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Out of memory while allocating room for a new SECTION \"%s\".\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }
  
  g_sec_tmp->priority = 0;
  g_sec_tmp->listfile_items = 0;
  g_sec_tmp->listfile_ints = NULL;
  g_sec_tmp->listfile_cmds = NULL;
  g_sec_tmp->maxsize_status = OFF;
  g_sec_tmp->data = NULL;
  g_sec_tmp->alignment = 1;
  g_sec_tmp->offset = 0;
  g_sec_tmp->advance_org = YES;
  g_sec_tmp->nspace = NULL;
  g_sec_tmp->keep = NO;

  if (strcmp(tmp, "BANKHEADER") == 0) {
    no_library_files("bank header sections");
      
    g_sec_next = g_sections_first;
    while (g_sec_next != NULL) {
      if (strcmp(g_sec_next->name, tmp) == 0 && g_sec_next->bank == g_bank) {
        snprintf(g_error_message, sizeof(g_error_message), "BANKHEADER section was defined for the second time for bank %d.\n", g_bank);
        print_error(g_error_message, ERROR_DIR);
        free(g_sec_tmp);
        return FAILED;
      }
      g_sec_next = g_sec_next->next;
    }
  }
  else {
    g_sec_next = g_sections_first;
    while (g_sec_next != NULL) {
      if (strcmp(g_sec_next->name, tmp) == 0) {
        snprintf(g_error_message, sizeof(g_error_message), "SECTION \"%s\" was defined for the second time.\n", tmp);
        print_error(g_error_message, ERROR_DIR);
        free(g_sec_tmp);
        return FAILED;
      }
      g_sec_next = g_sec_next->next;
    }
  }

  strcpy(g_sec_tmp->name, tmp);
  g_sec_tmp->next = NULL;

  g_sec_tmp->label_map = hashmap_new();

  if (g_sections_first == NULL) {
    g_sections_first = g_sec_tmp;
    g_sections_last = g_sec_tmp;
  }
  else {
    g_sections_last->next = g_sec_tmp;
    g_sections_last = g_sec_tmp;
  }

  if (compare_next_token("NAMESPACE") == SUCCEEDED) {
    struct namespace_def *nspace;

    if (skip_next_token() == FAILED)
      return FAILED;

    /* get the name */
    if (input_next_string() == FAILED)
      return FAILED;
    if (tmp[0] == '\"' && tmp[strlen(tmp)-1] == '\"') {
      l = 0;
      while (tmp[l+1] != '\"') {
        tmp[l] = tmp[l+1];
        l++;
      }
      tmp[l] = 0;
    }

    hashmap_get(g_namespace_map, tmp, (void*)&nspace);
    if (nspace == NULL) {
      nspace = calloc(1, sizeof(struct namespace_def));
      if (nspace == NULL) {
        print_error("Out of memory error.\n", ERROR_DIR);
        return FAILED;
      }
      strcpy(nspace->name, tmp);
      if (hashmap_put(g_namespace_map, nspace->name, nspace) != MAP_OK) {
        print_error("Namespace hashmap error.\n", ERROR_DIR);
        return FAILED;
      }
    }

    nspace->label_map = hashmap_new();
    g_sec_tmp->nspace = nspace;
  }

  /* add the namespace to the section's name? */
  if (g_active_file_info_last->namespace[0] != 0 && g_sec_tmp->nspace == NULL) {
    if (add_namespace_to_string(g_sec_tmp->name, sizeof(g_sec_tmp->name), "SECTION") == FAILED)
      return FAILED;
  }
  
  /* the size of the section? */
  if (compare_next_token("SIZE") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the SIZE.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_sec_tmp->maxsize_status == ON && g_sec_tmp->maxsize != d) {
      print_error("The SIZE of the section has already been defined.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->maxsize_status = ON;
    g_sec_tmp->maxsize = d;
  }

  /* align the section? */
  if (compare_next_token("ALIGN") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the .SECTION alignment.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->alignment = d;
  }

  /* offset the section? */
  if (compare_next_token("OFFSET") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the .SECTION offset.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->offset = d;
  }  

  /* the type of the section */
  if (compare_next_token("FORCE") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error("Libraries don't take FORCE sections.\n", ERROR_DIR);
      return FAILED;
    }
    g_sec_tmp->status = SECTION_STATUS_FORCE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("FREE") == SUCCEEDED) {
    g_sec_tmp->status = SECTION_STATUS_FREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("SUPERFREE") == SUCCEEDED) {
    g_sec_tmp->status = SECTION_STATUS_SUPERFREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("SEMIFREE") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error("Libraries don't take SEMIFREE sections.\n", ERROR_DIR);
      return FAILED;
    }
    g_sec_tmp->status = SECTION_STATUS_SEMIFREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("SEMISUBFREE") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error("Libraries don't take SEMISUBFREE sections.\n", ERROR_DIR);
      return FAILED;
    }
    g_sec_tmp->status = SECTION_STATUS_SEMISUBFREE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else if (compare_next_token("OVERWRITE") == SUCCEEDED) {
    if (g_output_format == OUTPUT_LIBRARY) {
      print_error("Libraries don't take OVERWRITE sections.\n", ERROR_DIR);
      return FAILED;
    }
    g_sec_tmp->status = SECTION_STATUS_OVERWRITE;
    if (skip_next_token() == FAILED)
      return FAILED;
  }
  else
    g_sec_tmp->status = SECTION_STATUS_FREE;

  /* return the org after the section? */
  if (compare_next_token("RETURNORG") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    g_sec_tmp->advance_org = NO;
  }

  if (compare_next_token("APPENDTO") == SUCCEEDED) {
    struct append_section *append_tmp;

    if (skip_next_token() == FAILED)
      return FAILED;

    append_tmp = calloc(sizeof(struct append_section), 1);
    if (append_tmp == NULL) {
      snprintf(g_error_message, sizeof(g_error_message), "Out of memory while allocating room for a new APPENDTO \"%s\".\n", tmp);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
      
    /* get the target section name */
    if (get_next_token() == FAILED) {
      free(append_tmp);
      return FAILED;
    }

    /* add the namespace to the section's name? */
    if (strlen(tmp) > 2 && tmp[0] == '*' && tmp[1] == ':') {
      char buf[MAX_NAME_LENGTH + 1];
      
      /* nope, this goes to global namespace. now '*:' has done its job, let's remove it */
      if (strlen(tmp) >= sizeof(buf)) {
        snprintf(g_error_message, sizeof(g_error_message), "The APPENDTO string \"%s\" is too long. Increase MAX_NAME_LENGTH in shared.h and recompile WLA.\n", tmp);
        print_error(g_error_message, ERROR_DIR);
        free(append_tmp);
        return FAILED;
      }

      strcpy(buf, &tmp[2]);
      strcpy(tmp, buf);
    }
    else if (g_active_file_info_last->namespace[0] != 0) {
      if (add_namespace_to_string(tmp, sizeof(tmp), "APPENDTO") == FAILED) {
        free(append_tmp);
        return FAILED;
      }
    }
    
    strcpy(append_tmp->section, g_sec_tmp->name);
    strcpy(append_tmp->append_to, tmp);

    append_tmp->next = g_append_sections;
    g_append_sections = append_tmp;
  }

  if (compare_next_token("PRIORITY") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    inz = input_number();
    if (inz != SUCCEEDED) {
      print_error("Could not parse the .SECTION priority.\n", ERROR_DIR);
      return FAILED;
    }

    g_sec_tmp->priority = d;
  }

  if (compare_next_token("KEEP") == SUCCEEDED) {
    if (skip_next_token() == FAILED)
      return FAILED;

    g_sec_tmp->keep = YES;
  }
  
  /* bankheader section? */
  if (strcmp(g_sec_tmp->name, "BANKHEADER") == 0) {
    g_sec_tmp->status = SECTION_STATUS_HEADER;
    g_bankheader_status = ON;
  }

  g_sec_tmp->id = g_section_id;
  g_sec_tmp->alive = ON;
  g_sec_tmp->filename_id = g_active_file_info_last->filename_id;
  g_sec_tmp->bank = g_bank;
  g_section_id++;
  g_section_status = ON;
  fprintf(g_file_out_ptr, "S%d ", g_sec_tmp->id);

  /* generate a section start label? */
  if (g_extra_definitions == ON)
    generate_label("SECTIONSTART_", g_sec_tmp->name);
  
  return SUCCEEDED;
}


int directive_fopen(void) {
  
  struct filepointer *f;
  char *c;
  int o;

  g_expect_calculations = NO;
  o = input_number();
  g_expect_calculations = YES;

  if (o != INPUT_NUMBER_STRING && o != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".FOPEN needs a file name string.\n", ERROR_DIR);
    return FAILED;
  }

  /* convert the path to local enviroment */
  localize_path(g_label);

  c = calloc(strlen(g_label) + 1, 1);
  if (c == NULL) {
    print_error("Out of memory error.\n", ERROR_DIR);
    return FAILED;
  }
  strcpy(c, g_label);

  /* get the file pointer name */
  if (get_next_token() == FAILED) {
    free(c);
    return FAILED;
  }

  /* is it defined already? */
  f = g_filepointers;
  while (f != NULL) {
    if (strcmp(tmp, f->name) == 0)
      break;
    f = f->next;
  }

  if (f != NULL) {
    /* it exists already! close the old handle and open the new one. */
    if (f->f != NULL) {
      fclose(f->f);
      f->f = NULL;
    }
    if (f->filename != NULL) {
      free(f->filename);
      f->filename = NULL;
    }
  }
  else {
    /* allocate a new filepointer */
    f = calloc(sizeof(struct filepointer), 1);
    if (f == NULL) {
      print_error("Out of memory error.\n", ERROR_DIR);
      free(c);
      return FAILED;
    }

    f->f = NULL;
    f->filename = NULL;
    f->next = g_filepointers;
    g_filepointers = f;
  }

  f->filename = c;
  strcpy(f->name, tmp);

  /* open the file */
  f->f = fopen(f->filename, "rb");
  if (f->f == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Error opening file \"%s\" for reading.\n", f->filename);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  return SUCCEEDED;
}


int directive_fclose(void) {
  
  struct filepointer *f, **t;

  /* get the file pointer name */
  if (get_next_token() == FAILED)
    return FAILED;

  f = g_filepointers;
  t = &g_filepointers;
  while (f != NULL) {
    if (strcmp(tmp, f->name) == 0)
      break;
    t = &(f->next);
    f = f->next;
  }

  if (f == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Couldn't find filepointer \"%s\".\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* close the file pointer */
  if (f->f != NULL) {
    fclose(f->f);
    f->f = NULL;
  }
  *t = f->next;

  free(f->filename);
  free(f);

  return SUCCEEDED;
}


int directive_fsize(void) {
  
  struct filepointer *f;
  long l, b;

  /* get the file pointer name */
  if (get_next_token() == FAILED)
    return FAILED;

  /* fetch the file pointer */
  f = g_filepointers;
  while (f != NULL) {
    if (strcmp(tmp, f->name) == 0)
      break;
    f = f->next;
  }

  if (f == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Couldn't find filepointer \"%s\".\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  l = ftell(f->f);
  fseek(f->f, 0, SEEK_END);
  b = ftell(f->f);
  fseek(f->f, l, SEEK_SET);

  /* get the definition label */
  if (get_next_token() == FAILED)
    return FAILED;

  if (add_a_new_definition(tmp, (double)b, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


int directive_fread(void) {
  
  struct filepointer *f;
  unsigned char c;

  /* get the file pointer name */
  if (get_next_token() == FAILED)
    return FAILED;

  f = g_filepointers;
  while (f != NULL) {
    if (strcmp(tmp, f->name) == 0)
      break;
    f = f->next;
  }

  if (f == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Couldn't find filepointer \"%s\".\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  fscanf(f->f, "%c", &c);

  /* get the definition label */
  if (get_next_token() == FAILED)
    return FAILED;

  if (redefine(tmp, (double)c, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


int directive_block(void) {

  struct block_name *b;
  
  if ((ind = get_next_token()) == FAILED)
    return FAILED;

  if (ind != GET_NEXT_TOKEN_STRING) {
    print_error(".BLOCK requires a name string.\n", ERROR_DIR);
    return FAILED;
  }

  b = calloc(sizeof(struct block_name), 1);
  if (b == NULL) {
    print_error("Out of memory while allocating block name.\n", ERROR_DIR);
    return FAILED;
  }

  strcpy(b->name, tmp);
  b->id = g_block_name_id++;
  b->next = g_block_names;
  g_block_names = b;
  
  g_block_status++;

  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);
  fprintf(g_file_out_ptr, "g%d ", b->id);

  return SUCCEEDED;
}


int directive_shift(void) {

  struct macro_argument *ma;
  struct macro_runtime *rt;
  struct macro_static *st;
  int q, o;

  if (g_macro_active == 0) {
    print_error(".SHIFT can only be used inside a MACRO.\n", ERROR_DIR);
    return FAILED;
  }

  rt = &g_macro_stack[g_macro_active - 1];
  st = rt->macro;

  if (st->nargument_names <= rt->supplied_arguments)
    o = st->nargument_names;
  else
    o = rt->supplied_arguments;

  /* free the argument definitions */
  for (q = 0; q < o; q++)
    undefine(st->argument_names[q]);

  /* free the first argument data */
  free(rt->argument_data[0]);

  /* shift the arguments one down */
  for (q = 0; q < rt->supplied_arguments - 1; q++)
    rt->argument_data[q] = rt->argument_data[q + 1];

  /* remove the last one */
  rt->argument_data[q] = NULL;
  rt->supplied_arguments--;

  if (st->nargument_names <= rt->supplied_arguments)
    o = st->nargument_names;
  else
    o = rt->supplied_arguments;

  /* redo the definitions if any */
  for (q = 0; q < o; q++) {
    ma = rt->argument_data[q];
    if (ma->type == SUCCEEDED)
      redefine(st->argument_names[q], ma->value, NULL, DEFINITION_TYPE_VALUE, 0);
    else if (ma->type == INPUT_NUMBER_STACK)
      redefine(st->argument_names[q], ma->value, NULL, DEFINITION_TYPE_STACK, 0);
    else if (ma->type == INPUT_NUMBER_ADDRESS_LABEL)
      redefine(st->argument_names[q], 0.0, ma->string, DEFINITION_TYPE_ADDRESS_LABEL, (int)strlen(ma->string));
    else if (ma->type == INPUT_NUMBER_STRING)
      redefine(st->argument_names[q], 0.0, ma->string, DEFINITION_TYPE_STRING, (int)strlen(ma->string));
  }

  /* redefine NARGS */
  if (redefine("NARGS", (double)rt->supplied_arguments, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;
  if (redefine("nargs", (double)rt->supplied_arguments, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
    return FAILED;

  return SUCCEEDED;
}


#ifdef GB

int directive_name_gb(void) {
      
  no_library_files(".NAME");
  
  if ((ind = get_next_token()) == FAILED)
    return FAILED;

  if (ind != GET_NEXT_TOKEN_STRING) {
    print_error(".NAME requires a string of 1 to 16 letters.\n", ERROR_DIR);
    return FAILED;
  }

  /* no name has been defined so far */
  if (g_name_defined == 0) {
    for (ind = 0; tmp[ind] != 0 && ind < 16; ind++)
      g_name[ind] = tmp[ind];

    if (ind == 16 && tmp[ind] != 0) {
      print_error(".NAME requires a string of 1 to 16 letters.\n", ERROR_DIR);
      return FAILED;
    }

    for ( ; ind < 16; g_name[ind] = 0, ind++)
      ;

    g_name_defined = 1;
  }
  /* compare the names */
  else {
    for (ind = 0; tmp[ind] != 0 && g_name[ind] != 0 && ind < 16; ind++)
      if (g_name[ind] != tmp[ind])
        break;

    if (ind == 16 && tmp[ind] != 0) {
      print_error(".NAME requires a string of 1 to 16 letters.\n", ERROR_DIR);
      return FAILED;
    }
    if (ind != 16 && (g_name[ind] != 0 || tmp[ind] != 0)) {
      print_error(".NAME was already defined.\n", ERROR_DIR);
      return FAILED;
    }
  }

  return SUCCEEDED;
}

#endif


int directive_rombanks(void) {

  int q;

  no_library_files(".ROMBANKS");
    
  if (g_banksize_defined == 0) {
    print_error("No .ROMBANKSIZE defined.\n", ERROR_DIR);
    return FAILED;
  }

  q = input_number();

  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED || d < 1) {
    print_error(".ROMBANKS needs a positive integer value.\n", ERROR_DIR);
    return FAILED;
  }

#ifdef GB
  if (d != 2 && d != 4 && d != 8 && d != 16 && d != 32 && d != 64 &&
      d != 128 && d != 256 && d != 512 && d != 72 && d != 80 && d != 96) {
    print_error("Unsupported amount of ROM banks.\n", ERROR_DIR);
    return FAILED;
  }

  if (d == 2)
    g_romtype = 0;
  else if (d == 4)
    g_romtype = 1;
  else if (d == 8)
    g_romtype = 2;
  else if (d == 16)
    g_romtype = 3;
  else if (d == 32)
    g_romtype = 4;
  else if (d == 64)
    g_romtype = 5;
  else if (d == 128)
    g_romtype = 6;
  else if (d == 256)
    g_romtype = 7;
  else if (d == 512)
    g_romtype = 8;
  else if (d == 72)
    g_romtype = 0x52;
  else if (d == 80)
    g_romtype = 0x53;
  else if (d == 96)
    g_romtype = 0x54;
#endif

  /* check that the old bank map (smaller) and the new one equal as much as they can */
  if (g_rombanks_defined != 0) {
    if (g_rombanks < d)
      inz = g_rombanks;
    else
      inz = d;

    for (ind = 0; ind < inz; ind++) {
      if (g_banks[ind] != g_banksize) {
        print_error("The old and the new .ROMBANKMAP's don't match.\n", ERROR_DIR);
        return FAILED;
      }
    }

    if (d <= g_rombanks)
      return SUCCEEDED;

    snprintf(g_error_message, sizeof(g_error_message), "Upgrading from %d to %d ROM banks.\n", g_rombanks, d);
    print_error(g_error_message, ERROR_WRN);
  }

  g_rombanks = d;
  g_rombanks_defined = 1;
  g_max_address = d * g_banksize;

  if (g_rom_banks != NULL)
    free(g_rom_banks);
  if (g_rom_banks_usage_table != NULL)
    free(g_rom_banks_usage_table);

  g_rom_banks = calloc(sizeof(unsigned char) * g_max_address, 1);
  g_rom_banks_usage_table = calloc(sizeof(unsigned char) * g_max_address, 1);
  if (g_rom_banks == NULL || g_rom_banks_usage_table == NULL) {
    print_error("Out of memory while allocating ROM banks.\n", ERROR_DIR);
    return FAILED;
  }

  memset(g_rom_banks_usage_table, 0, sizeof(unsigned char) * g_max_address);

  if (g_banks != NULL)
    free(g_banks);
  if (g_bankaddress != NULL)
    free(g_bankaddress);

  g_banks = calloc(sizeof(int) * g_rombanks, 1);
  g_bankaddress = calloc(sizeof(int) * g_rombanks, 1);
  if (g_banks == NULL || g_bankaddress == NULL) {
    print_error("Out of memory while allocating ROM banks.\n", ERROR_DIR);
    return FAILED;
  }

  for (inz = 0, ind = 0; ind < d; ind++) {
    g_banks[ind] = g_banksize;
    g_bankaddress[ind] = inz;
    inz += g_banksize;
  }

  return SUCCEEDED;
}


int directive_rombankmap(void) {
  
  int b = 0, a = 0, bt = 0, bt_defined = 0, x, bs = 0, bs_defined = 0, o, q;

  no_library_files(".ROMBANKMAP");

  /* ROMBANKMAP has been defined previously */
  if (g_rombankmap_defined != 0 || g_rombanks_defined != 0) {
    o = 0;
    while ((ind = get_next_token()) == SUCCEEDED) {
      /* .IF directive? */
      if (tmp[0] == '.') {
        q = parse_if_directive();
        if (q == FAILED)
          return FAILED;
        else if (q == SUCCEEDED)
          continue;
        /* else q == -1, continue executing below */
      }

      if (strcaselesscmp(tmp, ".ENDRO") == 0) {
        break;
      }
      else if (strcaselesscmp(tmp, "BANKSTOTAL") == 0) {
        q = input_number();
    
        if (q == FAILED)
          return FAILED;
        if (q != SUCCEEDED || d <= 0) {
          print_error("BANKSTOTAL needs a positive value.\n", ERROR_DIR);
          return FAILED;
        }

        if (g_rombanks < d) {
          g_banks = realloc(g_banks, sizeof(int) * d);
          g_bankaddress = realloc(g_bankaddress, sizeof(int) * d);
          if (g_banks == NULL || g_bankaddress == NULL) {
            print_error("Out of memory while allocating ROM banks.\n", ERROR_DIR);
            return FAILED;
          }
        }

        bt = d;
        bt_defined = 1;
      }
      else if (strcaselesscmp(tmp, "BANKSIZE") == 0) {
        if (bt_defined == 0) {
          print_error("BANKSTOTAL needs to be defined prior to BANKSIZE.\n", ERROR_DIR);
          return FAILED;
        }

        q = input_number();
    
        if (q == FAILED)
          return FAILED;
        if (q != SUCCEEDED || d <= 0) {
          print_error("BANKSIZE needs a positive value.\n", ERROR_DIR);
          return FAILED;
        }
    
        bs = d;
        bs_defined = 1;
      }
      else if (strcaselesscmp(tmp, "BANKS") == 0) {
        if (bs_defined == 0) {
          print_error("BANKSIZE needs to be defined prior to BANKS.\n", ERROR_DIR);
          return FAILED;
        }

        q = input_number();

        if (q == FAILED)
          return FAILED;
        if (q != SUCCEEDED || d <= 0) {
          print_error("BANKS needs a positive value.\n", ERROR_DIR);
          return FAILED;
        }

        for (x = 0; x < d; x++) {
          if (b > bt) {
            print_error("More BANKS than BANKSTOTAL suggests.\n", ERROR_DIR);
            return FAILED;
          }

          /* new banks? */
          if (x >= g_rombanks) {
            g_banks[o] = bs;
            g_bankaddress[o] = a;
          }
          /* compare old banks */
          else if (g_banks[o] != bs) {
            print_error("The old and the new ROMBANKMAPs don't match.\n", ERROR_DIR);
            return FAILED;
          }

          o++;
          b++;
          a += bs;
        }
      }
      else {
        ind = FAILED;
        break;
      }
    }
  }
  /* no ROMBANKMAP has been defined */
  else {
    o = 0;
    while ((ind = get_next_token()) == SUCCEEDED) {
      /* .IF directive? */
      if (tmp[0] == '.') {
        q = parse_if_directive();
        if (q == FAILED)
          return FAILED;
        else if (q == SUCCEEDED)
          continue;
        /* else q == -1, continue executing below */
      }
      
      if (strcaselesscmp(tmp, ".ENDRO") == 0)
        break;
      else if (strcaselesscmp(tmp, "BANKSTOTAL") == 0) {
        q = input_number();

        if (q == FAILED)
          return FAILED;
        if (q != SUCCEEDED || d <= 0) {
          print_error("BANKSTOTAL needs a positive value.\n", ERROR_DIR);
          return FAILED;
        }

        g_banks = calloc(sizeof(int) * d, 1);
        g_bankaddress = calloc(sizeof(int) * d, 1);
        if (g_banks == NULL || g_bankaddress == NULL) {
          print_error("Out of memory while allocating ROM banks.\n", ERROR_DIR);
          return FAILED;
        }

        bt = d;
        bt_defined = 1;
      }
      else if (strcaselesscmp(tmp, "BANKSIZE") == 0) {
        if (bt_defined == 0) {
          print_error("BANKSTOTAL needs to be defined prior to BANKSIZE.\n", ERROR_DIR);
          return FAILED;
        }

        q = input_number();

        if (q == FAILED)
          return FAILED;
        if (q != SUCCEEDED || d <= 0) {
          print_error("BANKSIZE needs a positive value.\n", ERROR_DIR);
          return FAILED;
        }

        bs = d;
        bs_defined = 1;
      }
      else if (strcaselesscmp(tmp, "BANKS") == 0) {
        if (bs_defined == 0) {
          print_error("BANKSIZE needs to be defined prior to BANKS.\n", ERROR_DIR);
          return FAILED;
        }

        q = input_number();

        if (q == FAILED)
          return FAILED;
        if (q != SUCCEEDED || d <= 0) {
          print_error("BANKS needs a positive value.\n", ERROR_DIR);
          return FAILED;
        }

        for (x = 0; x < d; x++) {
          if (b >= bt) {
            print_error("More BANKS than BANKSTOTAL suggests.\n", ERROR_DIR);
            return FAILED;
          }
          g_banks[o] = bs;
          g_bankaddress[o] = a;
          o++;
          b++;
          a += bs;
        }
      }
      else {
        ind = FAILED;
        break;
      }
    }
  }

  if (ind != SUCCEEDED) {
    print_error("Error in .ROMBANKMAP data structure.\n", ERROR_DIR);
    return FAILED;
  }

  /* no banks definded? */
  if (bt == 0) {
    print_error("No ROM banks were defined inside the .ROMBANKMAP.\n", ERROR_DIR);
    return FAILED;      
  }
  if (bt != b) {
    print_error("Not all ROM banks were defined inside the .ROMBANKMAP.\n", ERROR_DIR);
    return FAILED;      
  }

#ifdef GB
  if (b != 2 && b != 4 && b != 8 && b != 16 && b != 32 && b != 64 &&
      b != 128 && b != 256 && b != 512 && b != 72 && b != 80 && b != 96) {
    print_error("Unsupported amount of ROM banks.\n", ERROR_DIR);
    return FAILED;
  }

  if (b == 2)
    g_romtype = 0;
  else if (b == 4)
    g_romtype = 1;
  else if (b == 8)
    g_romtype = 2;
  else if (b == 16)
    g_romtype = 3;
  else if (b == 32)
    g_romtype = 4;
  else if (b == 64)
    g_romtype = 5;
  else if (b == 128)
    g_romtype = 6;
  else if (b == 256)
    g_romtype = 7;
  else if (b == 512)
    g_romtype = 8;
  else if (b == 72)
    g_romtype = 0x52;
  else if (b == 80)
    g_romtype = 0x53;
  else if (b == 96)
    g_romtype = 0x54;
#endif

  if (g_rombanks_defined != 0) {
    if (b > g_rombanks) {
      snprintf(g_error_message, sizeof(g_error_message), "Upgrading from %d to %d ROM banks.\n", g_rombanks, b);
      print_error(g_error_message, ERROR_WRN);
    }
    else
      return SUCCEEDED;
  }

  g_rombanks = b;
  g_rombanks_defined = 1;
  for (g_max_address = 0, q = 0; q < g_rombanks; q++)
    g_max_address += g_banks[q];

  if (g_rom_banks != NULL)
    free(g_rom_banks);
  if (g_rom_banks_usage_table != NULL)
    free(g_rom_banks_usage_table);

  g_rom_banks = calloc(sizeof(unsigned char) * g_max_address, 1);
  g_rom_banks_usage_table = calloc(sizeof(unsigned char) * g_max_address, 1);
  if (g_rom_banks == NULL || g_rom_banks_usage_table == NULL) {
    print_error("Out of memory while allocating ROM banks.\n", ERROR_DIR);
    return FAILED;
  }

  memset(g_rom_banks_usage_table, 0, sizeof(unsigned char) * g_max_address);
  g_rombankmap_defined = 1;

  return SUCCEEDED;
}


int directive_memorymap(void) {
  
  int slotsize = 0, slotsize_defined = 0, s = 0, q, o;

  if (g_memorymap_defined == 1) {
    print_error(".MEMORYMAP can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_output_format == OUTPUT_LIBRARY)
    print_error("Libraries don't need .MEMORYMAP.\n", ERROR_WRN);

  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDME") == 0) {
      if (g_defaultslot_defined == 0) {
        print_error("No DEFAULTSLOT defined.\n", ERROR_DIR);
        return FAILED;
      }

      if (g_slots[g_defaultslot].size == 0) {
        snprintf(g_error_message, sizeof(g_error_message), "Unknown DEFAULTSLOT %d.\n", g_defaultslot);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      break;
    }
    else if (strcaselesscmp(tmp, "SLOTSIZE") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED) {
        print_error("SLOTSIZE needs an immediate value.\n", ERROR_DIR);
        return FAILED;
      }

      slotsize = d;
      slotsize_defined = 1;
    }
    else if (strcaselesscmp(tmp, "DEFAULTSLOT") == 0) {
      if (g_defaultslot_defined != 0) {
        print_error("DEFAULTSLOT can be defined only once.\n", ERROR_DIR);
        return FAILED;
      }

      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d > 255 || d < 0) {
        print_error("DEFAULTSLOT needs an immediate value (0-255) as an ID.\n", ERROR_DIR);
        return FAILED;
      }

      g_defaultslot_defined = 1;
      g_defaultslot = d;
    }
    else if (strcaselesscmp(tmp, "SLOT") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d > 255 || d < 0) {
        print_error("SLOT needs a positive value (0-255) as an ID.\n", ERROR_DIR);
        return FAILED;
      }

      if (s != d) {
        print_error("Error in SLOT's ID. ID's start from 0.\n", ERROR_DIR);
        return FAILED;
      }

      o = d;

      /* skip "START" if present */
      if (compare_next_token("START") == SUCCEEDED)
        skip_next_token();

      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d < 0) {
        print_error("The starting address has to be a non-negative value.\n", ERROR_DIR);
        return FAILED;
      }

      g_slots[o].address = d;

      /* skip "SIZE" if present */
      if (compare_next_token("SIZE") == SUCCEEDED)
        skip_next_token();

      q = input_number();

      if (q == INPUT_NUMBER_EOL) {
        if (slotsize_defined == 0) {
          print_error("SLOTSIZE must be defined if you don't explicitly give the size.\n", ERROR_DIR);
          return FAILED;
        }

        next_line();
    
        d = slotsize;
      }
      else {
        if (q == FAILED)
          return FAILED;
        if (q == INPUT_NUMBER_ADDRESS_LABEL || q == INPUT_NUMBER_STRING) {
          /* we got the name for the SLOT instead of its SIZE */
          strcpy(g_slots[o].name, g_label);
          d = slotsize;
        }
        else if (q != SUCCEEDED) {
          print_error("The size of the slot needs to be an immediate value.\n", ERROR_DIR);
          return FAILED;
        }
      }
      
      g_slots[o].size = d;

      if (q != INPUT_NUMBER_EOL) {
        /* skip "NAME" if present */
        if (compare_next_token("NAME") == SUCCEEDED)
          skip_next_token();

        q = input_number();

        if (q == INPUT_NUMBER_EOL)
          next_line();
        else {
          if (q != INPUT_NUMBER_ADDRESS_LABEL && q != INPUT_NUMBER_STRING) {
            print_error("NAME needs a label/string for name.\n", ERROR_DIR);
            return FAILED;      
          }

          strcpy(g_slots[o].name, g_label);
        }
      }
      
      g_slots_amount++;
      s++;
    }
    else {
      ind = FAILED;
      break;
    }
  }

  if (ind != SUCCEEDED) {
    print_error("Error in .MEMORYMAP data structure.\n", ERROR_DIR);
    return FAILED;
  }

  g_memorymap_defined = 1;

  return SUCCEEDED;
}


int directive_unbackground(void) {
  
  int start, end, q;

  if (g_output_format != OUTPUT_OBJECT) {
    print_error(".UNBACKGROUND can only be used in OBJECT output mode.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_background_defined == 0) {
    print_error("No .BACKGROUND specified.\n", ERROR_DIR);
    return FAILED;
  }

  /* get the starting address */
  q = input_number();

  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED || q < 0) {
    print_error(".UNBACKGROUND needs the block's starting address.\n", ERROR_DIR);
    return FAILED;
  }

  start = d;

  /* get the ending address */
  q = input_number();

  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED || q < 0) {
    print_error(".UNBACKGROUND needs the block's ending address.\n", ERROR_DIR);
    return FAILED;
  }

  end = d;

  if (end < start) {
    print_error("The block's ending address is smaller than the starting address!\n", ERROR_DIR);
    return FAILED;
  }
  if (start >= g_max_address) {
    snprintf(g_error_message, sizeof(g_error_message), "The block's starting address $%x is beyond the ROM's ending address $%x.\n", start, g_max_address-1);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }
  if (end >= g_max_address) {
    snprintf(g_error_message, sizeof(g_error_message), "The block's ending address $%x is beyond the ROM's ending address $%x. Using the ROM's ending address instead.\n", end, g_max_address-1);
    end = g_max_address;
    print_error(g_error_message, ERROR_WRN);
  }
  
  /* clear the memory [start, end] */
  memset(g_rom_banks + start, 0, end - start + 1);
  memset(g_rom_banks_usage_table + start, 0, end - start + 1);

  return SUCCEEDED;
}


int directive_background(void) {
  
  FILE *file_in_ptr;
  int q;

  if (g_output_format != OUTPUT_OBJECT) {
    print_error(".BACKGROUND can only be used in OBJECT output mode.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_background_defined == 1) {
    print_error("Only one .BACKGROUND can be specified.\n", ERROR_DIR);
    return FAILED;
  }

  g_expect_calculations = NO;
  q = input_number();
  g_expect_calculations = YES;

  if (q != INPUT_NUMBER_STRING && q != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".BACKGROUND needs a file name string.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_memorymap_defined == 0) {
    print_error("No .MEMORYMAP defined.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_rombanks_defined == 0) {
    print_error("No .ROMBANKS defined.\n", ERROR_DIR);
    return FAILED;
  }

  create_full_name(g_include_dir, g_label);

  if ((file_in_ptr = fopen(g_full_name, "rb")) == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Error opening .BACKGROUND file \"%s\".\n", g_full_name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  fseek(file_in_ptr, 0, SEEK_END);
  g_background_size = (int)ftell(file_in_ptr);
  fseek(file_in_ptr, 0, SEEK_SET);
  
  if (g_background_size > g_max_address) {
    snprintf(g_error_message, sizeof(g_error_message), ".BACKGROUND file \"%s\" size (%d) is larger than ROM size (%d).\n", g_full_name, g_background_size, g_max_address);
    print_error(g_error_message, ERROR_DIR);
    fclose(file_in_ptr);
    return FAILED;
  }

  memset(g_rom_banks_usage_table, 2, g_background_size);
  fread(g_rom_banks, 1, g_max_address, file_in_ptr);

  g_background_defined = 1;
  fclose(file_in_ptr);

  return SUCCEEDED;
}


#ifdef GB

int directive_gbheader(void) {

  int q;
    
  if (g_gbheader_defined != 0) {
    print_error(".GBHEADER can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_computechecksum_defined != 0)
    print_error(".COMPUTEGBCHECKSUM is unnecessary when .GBHEADER is defined.\n", ERROR_WRN);
  else
    g_computechecksum_defined++;

  if (g_computecomplementcheck_defined != 0)
    print_error(".COMPUTEGBCOMPLEMENTCHECK is unnecessary when .GBHEADER is defined.\n", ERROR_WRN);
  else
    g_computecomplementcheck_defined++;

  if (g_output_format == OUTPUT_LIBRARY) {
    print_error("Libraries don't take .GBHEADER.\n", ERROR_DIR);
    return FAILED;
  }

  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDGB") == 0)
      break;
    else if (strcaselesscmp(tmp, "NINTENDOLOGO") == 0)
      g_nintendologo_defined++;
    else if (strcaselesscmp(tmp, "ROMDMG") == 0) {
      if (g_romgbc == 1) {
        print_error(".ROMGBC was defined prior to .ROMDMG.\n", ERROR_DIR);
        return FAILED;
      }
      else if (g_romgbc == 2) {
        print_error(".ROMGBCONLY was defined prior to .ROMDMG.\n", ERROR_DIR);
        return FAILED;
      }
      else if (g_romsgb != 0) {
        print_error(".ROMDMG and .ROMSGB cannot be mixed.\n", ERROR_DIR);
        return FAILED;
      }
      g_romdmg = 1;
    }
    else if (strcaselesscmp(tmp, "ROMGBC") == 0) {
      if (g_romdmg != 0) {
        print_error(".ROMDMG was defined prior to .ROMGBC.\n", ERROR_DIR);
        return FAILED;
      }
      else if (g_romgbc == 2) {
        print_error(".ROMGBCONLY was defined prior to .ROMGBC.\n", ERROR_DIR);
        return FAILED;
      }
      g_romgbc = 1;
    }
    else if (strcaselesscmp(tmp, "ROMGBCONLY") == 0) {
      if (g_romdmg != 0) {
        print_error(".ROMDMG was defined prior to .ROMGBCONLY.\n", ERROR_DIR);
        return FAILED;
      }
      else if (g_romgbc == 1) {
        print_error(".ROMGBC was defined prior to .ROMGBCONLY.\n", ERROR_DIR);
        return FAILED;
      }
      g_romgbc = 2;
    }
    else if (strcaselesscmp(tmp, "ROMSGB") == 0) {
      if (g_romdmg != 0) {
        print_error(".ROMDMG and .ROMSGB cannot be mixed.\n", ERROR_DIR);
        return FAILED;
      }
      g_romsgb++;
    }
    else if (strcaselesscmp(tmp, "NAME") == 0) {
      if ((ind = get_next_token()) == FAILED)
        return FAILED;

      if (ind != GET_NEXT_TOKEN_STRING) {
        print_error("NAME requires a string of 1 to 16 letters.\n", ERROR_DIR);
        return FAILED;
      }

      /* no name has been defined so far */
      if (g_name_defined == 0) {
        for (ind = 0; tmp[ind] != 0 && ind < 16; ind++)
          g_name[ind] = tmp[ind];
    
        if (ind == 16 && tmp[ind] != 0) {
          print_error("NAME requires a string of 1 to 16 letters.\n", ERROR_DIR);
          return FAILED;
        }

        for ( ; ind < 16; g_name[ind] = 0, ind++)
          ;

        g_name_defined = 1;
      }
      else {
        /* compare the names */
        for (ind = 0; tmp[ind] != 0 && g_name[ind] != 0 && ind < 16; ind++)
          if (g_name[ind] != tmp[ind])
            break;
    
        if (ind == 16 && tmp[ind] != 0) {
          print_error("NAME requires a string of 1 to 16 letters.\n", ERROR_DIR);
          return FAILED;
        }
        if (ind != 16 && (g_name[ind] != 0 || tmp[ind] != 0)) {
          print_error("NAME was already defined.\n", ERROR_DIR);
          return FAILED;
        }
      }
    }
    else if (strcaselesscmp(tmp, "LICENSEECODEOLD") == 0) {
      if (g_licenseecodenew_defined != 0) {
        print_error(".LICENSEECODENEW and .LICENSEECODEOLD cannot both be defined.\n", ERROR_DIR);
        return FAILED;
      }

      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d < -128 || d > 255) {
        snprintf(g_error_message, sizeof(g_error_message), ".LICENSEECODEOLD needs a 8-bit value, got %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (g_licenseecodeold_defined != 0) {
        if (g_licenseecodeold != d) {
          print_error(".LICENSEECODEOLD was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }
      }

      g_licenseecodeold = d;
      g_licenseecodeold_defined = 1;
    }
    else if (strcaselesscmp(tmp, "LICENSEECODENEW") == 0) {
      if (g_licenseecodeold_defined != 0) {
        print_error(".LICENSEECODENEW and .LICENSEECODEOLD cannot both be defined.\n", ERROR_DIR);
        return FAILED;
      }

      if ((ind = get_next_token()) == FAILED)
        return FAILED;

      if (ind != GET_NEXT_TOKEN_STRING) {
        print_error(".LICENSEECODENEW requires a string of two letters.\n", ERROR_DIR);
        return FAILED;
      }
      if (!(tmp[0] != 0 && tmp[1] != 0 && tmp[2] == 0)) {
        print_error(".LICENSEECODENEW requires a string of two letters.\n", ERROR_DIR);
        return FAILED;
      }

      if (g_licenseecodenew_defined != 0) {
        if (tmp[0] != g_licenseecodenew_c1 || tmp[1] != g_licenseecodenew_c2) {
          print_error(".LICENSEECODENEW was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }
      }

      g_licenseecodenew_c1 = tmp[0];
      g_licenseecodenew_c2 = tmp[1];
      g_licenseecodenew_defined = 1;
    }
    else if (strcaselesscmp(tmp, "CARTRIDGETYPE") == 0) {
      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "CARTRIDGETYPE needs a 8-bit value, got %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_cartridgetype_defined != 0 && g_cartridgetype != d) {
          print_error("CARTRIDGETYPE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_cartridgetype = d;
        g_cartridgetype_defined = 1;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "RAMSIZE") == 0) {
      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "RAMSIZE needs a 8-bit value, got %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_rambanks_defined != 0 && g_rambanks != d) {
          print_error("RAMSIZE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_rambanks = d;
        g_rambanks_defined = 1;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "COUNTRYCODE") == 0) {
      inz = input_number();
      
      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "COUNTRYCODE needs a non-negative value, got %d.\n\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_countrycode_defined != 0 && g_countrycode != d) {
          print_error("COUNTRYCODE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_countrycode = d;
        g_countrycode_defined = 1;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "DESTINATIONCODE") == 0) {
      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "DESTINATIONCODE needs a non-negative value, got %d.\n\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_countrycode_defined != 0 && g_countrycode != d) {
          print_error("DESTINATIONCODE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_countrycode = d;
        g_countrycode_defined = 1;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "VERSION") == 0) {
      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "VERSION needs a non-negative value, got %d.\n\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_version_defined != 0 && g_version != d) {
          print_error("VERSION was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_version = d;
        g_version_defined = 1;
      }
      else
        return FAILED;
    }
    else {
      ind = FAILED;
      break;
    }
  }

  if (ind != SUCCEEDED) {
    print_error("Error in .GBHEADER data structure.\n", ERROR_DIR);
    return FAILED;
  }

  g_gbheader_defined = 1;

  return SUCCEEDED;
}

#endif


int directive_define_def_equ(void) {
  
  int j, g_size, export, q;
  double dou;
  char k[256];

  if (get_next_plain_string() == FAILED)
    return FAILED;

  /* check the user doesn't try to define reserved labels */
  if (is_reserved_definition(tmp) == YES) {
    snprintf(g_error_message, sizeof(g_error_message), "\"%s\" is a reserved definition label and is not user definable.\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* skip "=", if present */
  if (compare_next_token("=") == SUCCEEDED)
    skip_next_token();

  g_input_float_mode = ON;
  q = get_new_definition_data(&j, k, &g_size, &dou, &export);
  g_input_float_mode = OFF;
  if (q == FAILED)
    return FAILED;

  if (!(q == INPUT_NUMBER_EOL || q == INPUT_NUMBER_FLOAT || q == SUCCEEDED || q == INPUT_NUMBER_STRING || q == INPUT_NUMBER_STACK)) {
    print_error("Could not parse the definition data.\n", ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED)
    q = add_a_new_definition(tmp, (double)j, NULL, DEFINITION_TYPE_VALUE, 0);
  else if (q == INPUT_NUMBER_FLOAT)
    q = add_a_new_definition(tmp, dou, NULL, DEFINITION_TYPE_VALUE, 0);
  else if (q == INPUT_NUMBER_STRING)
    q = add_a_new_definition(tmp, 0.0, k, DEFINITION_TYPE_STRING, g_size);
  else if (q == INPUT_NUMBER_STACK)
    q = add_a_new_definition(tmp, (double)j, NULL, DEFINITION_TYPE_STACK, 0);
  else if (q == INPUT_NUMBER_EOL)
    q = add_a_new_definition(tmp, 0.0, NULL, DEFINITION_TYPE_VALUE, 0);
  
  if (q == FAILED)
    return FAILED;

  if (export == YES) {
    if (export_a_definition(tmp) == FAILED)
      return FAILED;
  }

  return SUCCEEDED;
}


int directive_undef_undefine(void) {

  char bak[256];
  int q;

  strcpy(bak, cp);

  q = 0;
  while (1) {
    ind = input_next_string();
    if (ind == FAILED)
      return FAILED;
    if (ind == INPUT_NUMBER_EOL) {
      if (q != 0) {
        next_line();
        return SUCCEEDED;
      }
      snprintf(g_error_message, sizeof(g_error_message), ".%s requires definition name(s).\n", bak);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    q++;

    if (undefine(tmp) == FAILED) {
      snprintf(g_error_message, sizeof(g_error_message), "Could not .%s \"%s\".\n", bak, tmp);
      print_error(g_error_message, ERROR_WRN);
    }
  }

  return SUCCEEDED;
}


int directive_enumid(void) {
  
  int q;
  
  q = input_number();

  if (q == FAILED)
    return FAILED;

  if (q == INPUT_NUMBER_EOL) {
    print_error(".ENUMID needs a value or a definition name.\n", ERROR_DIR);
    return FAILED;
  }
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    if (g_enumid_defined == 0) {
      print_error(".ENUMID needs the initial value when .ENUMID is used the first time.\n", ERROR_DIR);
      return FAILED;
    }
    
    if (is_reserved_definition(g_label) == YES) {
      snprintf(g_error_message, sizeof(g_error_message), "\"%s\" is a reserved definition label and is not user definable.\n", g_label);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (add_a_new_definition(g_label, (double)g_enumid, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
      return FAILED;

    if (g_enumid_export == 1) {
      if (export_a_definition(g_label) == FAILED)
        return FAILED;
    }

    g_enumid += g_enumid_adder;

    return SUCCEEDED;
  }
  else if (q == SUCCEEDED) {
    g_enumid = d;
    g_enumid_adder = 1;
    g_enumid_export = 0;

    if (compare_next_token("STEP") == SUCCEEDED) {
      skip_next_token();

      q = input_number();

      if (q == FAILED)
        return FAILED;

      if (q != SUCCEEDED) {
        print_error("STEP needs a value\n", ERROR_DIR);
        return FAILED;
      }

      g_enumid_adder = d;
    }

    if (compare_next_token("EXPORT") == SUCCEEDED) {
      skip_next_token();

      g_enumid_export = 1;
    }

    g_enumid_defined = 1;

    return SUCCEEDED;
  }
  else {
    print_error(".ENUMID needs a value or a definition name.\n", ERROR_DIR);
    return FAILED;
  }
}


int directive_input(void) {
  
  char k[256];
  int j, v;

  if (get_next_token() == FAILED)
    return FAILED;

  fgets(k, 254, stdin);

  for (j = 0; j < 254; j++) {
    if (k[j] == 0)
      break;
    if (k[j] == 0x0A) {
      k[j] = 0;
      break;
    }
    if (k[j] == 0x0D) {
      k[j] = 0;
      break;
    }
  }

  if (j == 254) {
    print_error("Error in .INPUT.\n", ERROR_DIR);
    return FAILED;
  }

  for (j = 0; j < 254; j++) {
    if (k[j] == 0) {
      print_error("No .INPUT?\n", ERROR_DIR);
      return FAILED;
    }
    if (!(k[j] == ' ' || k[j] == 0x09))
      break;
  }

  if (k[j] == '%') {
    v = 0;
    j++;
    for ( ; j < 254; j++) {
      if (k[j] == 0)
        break;
      if (k[j] == '0' || k[j] == '1')
        v = (v << 1) + k[j] - '0';
      else
        break;
    }
    if (k[j] == 0) {
      redefine(tmp, (double)v, NULL, DEFINITION_TYPE_VALUE, 0);
      return SUCCEEDED;
    }
  }
  else if (k[j] == '$') {
    j++;
    v = 0;
    for ( ; j < 254; j++) {
      if (k[j] == 0)
        break;
      if (k[j] >= '0' && k[j] <= '9')
        v = (v << 4) + k[j] - '0';
      else if (k[j] >= 'a' && k[j] <= 'f')
        v = (v << 4) + k[j] - 'a' + 10;
      else if (k[j] >= 'A' && k[j] <= 'F')
        v = (v << 4) + k[j] - 'A' + 10;
      else
        break;
    }
    if (k[j] == 0) {
      redefine(tmp, (double)v, NULL, DEFINITION_TYPE_VALUE, 0);
      return SUCCEEDED;
    }
  }
  else if (k[j] >= '0' && k[j] <= '9') {
    v = 0;
    for ( ; j < 254; j++) {
      if (k[j] == 0)
        break;
      if (k[j] >= '0' && k[j] <= '9')
        v = (v * 10) + k[j] - '0';
      else
        break;
    }
    if (k[j] == 0) {
      redefine(tmp, (double)v, NULL, DEFINITION_TYPE_VALUE, 0);
      return SUCCEEDED;
    }
  }

  /* it's a string */
  redefine(tmp, 0.0, k, DEFINITION_TYPE_STRING, (int)strlen(k));

  return SUCCEEDED;
}


int directive_redefine_redef(void) {
  
  int j, g_size, export, q;
  double dou;
  char k[256];

  if (get_next_plain_string() == FAILED)
    return FAILED;

  /* check the user doesn't try to define reserved labels */
  if (is_reserved_definition(tmp) == YES) {
    snprintf(g_error_message, sizeof(g_error_message), "\"%s\" is a reserved definition label and is not user definable.\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* skip "=", if present */
  if (compare_next_token("=") == SUCCEEDED)
    skip_next_token();

  g_input_float_mode = ON;
  q = get_new_definition_data(&j, k, &g_size, &dou, &export);
  g_input_float_mode = OFF;
  if (q == FAILED)
    return FAILED;

  if (!(q == INPUT_NUMBER_FLOAT || q == SUCCEEDED || q == INPUT_NUMBER_STRING || q == INPUT_NUMBER_STACK)) {
    print_error("Could not parse the definition data.\n", ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED)
    redefine(tmp, (double)j, NULL, DEFINITION_TYPE_VALUE, 0);
  else if (q == INPUT_NUMBER_FLOAT)
    redefine(tmp, dou, NULL, DEFINITION_TYPE_VALUE, 0);
  else if (q == INPUT_NUMBER_STRING)
    redefine(tmp, 0.0, k, DEFINITION_TYPE_STRING, g_size);
  else if (q == INPUT_NUMBER_STACK)
    redefine(tmp, (double)j, NULL, DEFINITION_TYPE_STACK, 0);

  if (export == YES) {
    if (export_a_definition(tmp) == FAILED)
      return FAILED;
  }
    
  return SUCCEEDED;
}


#ifdef Z80

int directive_smsheader(void) {
  
  int q;
    
  if (g_smsheader_defined != 0) {
    print_error(".SMSHEADER can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_computesmschecksum_defined != 0)
    print_error(".COMPUTESMSCHECKSUM is unnecessary when .SMSHEADER is defined.\n", ERROR_WRN);

  if (g_smstag_defined != 0)
    print_error(".SMSTAG is unnecessary when .SMSHEADER is defined.\n", ERROR_WRN);

  if (g_output_format == OUTPUT_LIBRARY) {
    print_error("Libraries don't take .SMSHEADER.\n", ERROR_DIR);
    return FAILED;
  }

  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDSMS") == 0)
      break;
    else if (strcaselesscmp(tmp, "VERSION") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d < 0 || d > 15) {
        snprintf(g_error_message, sizeof(g_error_message), "VERSION needs a value between 0 and 15, got %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (g_smsversion_defined != 0) {
        if (g_smsversion != d) {
          print_error("VERSION was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }
      }

      g_smsversion = d;
      g_smsversion_defined = 1;
    }
    else if (strcaselesscmp(tmp, "ROMSIZE") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d < 0 || d > 15) {
        snprintf(g_error_message, sizeof(g_error_message), "ROMSIZE needs a value between 0 and 15, got %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (g_smsromsize_defined != 0) {
        if (g_smsromsize != d) {
          print_error("ROMSIZE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }
      }

      g_smsromsize = d;
      g_smsromsize_defined = 1;
    }
    else if (strcaselesscmp(tmp, "REGIONCODE") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED || d < 3|| d > 7) {
        snprintf(g_error_message, sizeof(g_error_message), "REGIONCODE needs a value between 3 and 7, got %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (g_smsregioncode_defined != 0) {
        if (g_smsregioncode != d) {
          print_error("REGIONCODE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }
      }

      g_smsregioncode = d;
      g_smsregioncode_defined = 1;
    }
    else if (strcaselesscmp(tmp, "PRODUCTCODE") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED) {
        print_error("PRODUCTCODE needs 2.5 bytes of data.\n", ERROR_DIR);
        return FAILED;
      }

      g_smsproductcode1 = d & 255;
      g_smsproductcode_defined = 1;

      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED) {
        print_error("PRODUCTCODE needs 2.5 bytes of data.\n", ERROR_DIR);
        return FAILED;
      }

      g_smsproductcode2 = d & 255;

      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED) {
        print_error("PRODUCTCODE needs 2.5 bytes of data.\n", ERROR_DIR);
        return FAILED;
      }

      g_smsproductcode3 = d & 15;
    }
    else if (strcaselesscmp(tmp, "RESERVEDSPACE") == 0) {
      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED) {
        print_error("RESERVEDSPACE needs 2 bytes of data.\n", ERROR_DIR);
        return FAILED;
      }

      g_smsreservedspace1 = d & 255;
      smsreservedspace_defined = 1;

      q = input_number();

      if (q == FAILED)
        return FAILED;
      if (q != SUCCEEDED) {
        print_error("RESERVEDSPACE needs 2 bytes of data.\n", ERROR_DIR);
        return FAILED;
      }

      g_smsreservedspace2 = d & 255;
    }
    else {
      ind = FAILED;
      break;
    }
  }

  if (ind != SUCCEEDED) {
    print_error("Error in .SMSHEADER data structure.\n", ERROR_DIR);
    return FAILED;
  }

  g_smsheader_defined = 1;

  return SUCCEEDED;
}


int directive_sdsctag(void) {
  
  int q;

  if (g_sdsctag_defined != 0) {
    print_error(".SDSCTAG can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }
    
  no_library_files(".SDSCTAG");

  g_input_float_mode = ON;
  q = input_number();
  g_input_float_mode = OFF;
  if (q != SUCCEEDED && q != INPUT_NUMBER_FLOAT) {
    print_error(".SDSCTAG needs the version number.\n" , ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED) {
    g_sdsc_ma = d;
    g_sdsc_mi = 0;
  }
  else {
    g_sdsc_ma = (int)g_parsed_double;
    g_sdsc_mi = g_parsed_double_decimal_numbers;
  }
  
  if (g_sdsc_ma >= 100 || g_sdsc_mi >= 100) {
    print_error(".SDSCTAG major and minor version numbers must be inside the range [0,99].\n" , ERROR_DIR);
    return FAILED;
  }

  /* program name */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (!(q == INPUT_NUMBER_STRING || q == SUCCEEDED || q == INPUT_NUMBER_STACK || q == INPUT_NUMBER_ADDRESS_LABEL)) {
    print_error(".SDSCTAG requires program name string (or a pointer to it).\n", ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED) {
    g_sdsctag_name_type = TYPE_VALUE;
    g_sdsctag_name_value = d;
  }
  else if (q == INPUT_NUMBER_STRING) {
    if (g_label[0] == 0) {
      g_sdsctag_name_type = TYPE_VALUE;
      g_sdsctag_name_value = 0xFFFF; /* null string */
    }
    else {
      g_sdsctag_name_type = TYPE_STRING;
      strcpy(g_sdsctag_name_str, g_label);
    }
  }
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    g_sdsctag_name_type = TYPE_LABEL;
    strcpy(g_sdsctag_name_str, g_label);
  }
  else {
    g_sdsctag_name_type = TYPE_STACK_CALCULATION;
    g_sdsctag_name_value = g_latest_stack;
  }

  /* notes */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (!(q == INPUT_NUMBER_STRING || q == SUCCEEDED || q == INPUT_NUMBER_STACK || q == INPUT_NUMBER_ADDRESS_LABEL)) {
    print_error(".SDSCTAG requires program release notes string (or a pointer to it).\n", ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED) {
    g_sdsctag_notes_type = TYPE_VALUE;
    g_sdsctag_notes_value = d;
  }
  else if (q == INPUT_NUMBER_STRING) {
    if (g_label[0] == 0) {
      g_sdsctag_notes_type = TYPE_VALUE;
      g_sdsctag_notes_value = 0xFFFF; /* null string */
    }
    else {
      g_sdsctag_notes_type = TYPE_STRING;
      strcpy(g_sdsctag_notes_str, g_label);
    }
  }
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    g_sdsctag_notes_type = TYPE_LABEL;
    strcpy(g_sdsctag_notes_str, g_label);
  }
  else {
    g_sdsctag_notes_type = TYPE_STACK_CALCULATION;
    g_sdsctag_notes_value = g_latest_stack;
  }

  /* program author */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (!(q == INPUT_NUMBER_STRING || q == SUCCEEDED || q == INPUT_NUMBER_STACK || q == INPUT_NUMBER_ADDRESS_LABEL)) {
    print_error(".SDSCTAG requires program author string (or a pointer to it).\n", ERROR_DIR);
    return FAILED;
  }

  if (q == SUCCEEDED) {
    g_sdsctag_author_type = TYPE_VALUE;
    g_sdsctag_author_value = d;
  }
  else if (q == INPUT_NUMBER_STRING) {
    if (g_label[0] == 0) {
      g_sdsctag_author_type = TYPE_VALUE;
      g_sdsctag_author_value = 0xFFFF; /* null string */
    }
    else {
      g_sdsctag_author_type = TYPE_STRING;
      strcpy(g_sdsctag_author_str, g_label);
    }
  }
  else if (q == INPUT_NUMBER_ADDRESS_LABEL) {
    g_sdsctag_author_type = TYPE_LABEL;
    strcpy(g_sdsctag_author_str, g_label);
  }
  else {
    g_sdsctag_author_type = TYPE_STACK_CALCULATION;
    g_sdsctag_author_value = g_latest_stack;
  }

  g_sdsctag_defined++;
  g_smstag_defined++;
  g_computesmschecksum_defined++;

  return SUCCEEDED;
}

#endif


int directive_macro(void) {

  struct macro_static *m;
  int macro_start_line;
  int q;

  if (g_dstruct_status == ON) {
    print_error("You can't define a macro inside .DSTRUCT.\n", ERROR_DIR);
    return FAILED;
  }

  if (get_next_token() == FAILED)
    return FAILED;

  if (strcaselesscmp(cp, "ENDM") == 0) {
    print_error("A MACRO must have a name.\n", ERROR_DIR);
    return FAILED;
  }

  macro_start_line = g_active_file_info_last->line_current;

  /* append the namespace, if this file uses if */
  if (g_active_file_info_last->namespace[0] != 0) {
    if (add_namespace_to_string(tmp, sizeof(tmp), "MACRO") == FAILED)
      return FAILED;
  }

  if (macro_get(tmp, NO, &m) == FAILED)
    return FAILED;
  
  if (m != NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "MACRO \"%s\" was defined for the second time.\n", tmp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  m = calloc(sizeof(struct macro_static), 1);
  if (m == NULL) {
    print_error("Out of memory while allocating room for a new MACRO.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_macros_first == NULL) {
    g_macros_first = m;
    g_macros_last = m;
  }
  else {
    g_macros_last->next = m;
    g_macros_last = m;
  }

  strcpy(m->name, tmp);
  m->next = NULL;
  m->calls = 0;
  m->filename_id = g_active_file_info_last->filename_id;
  m->argument_names = NULL;

  /* is ARGS defined? */
  q = 0;
  if (compare_next_token("ARGS") == SUCCEEDED) {
    skip_next_token();

    while (1) {
      ind = input_next_string();
      if (ind == FAILED)
        return FAILED;
      if (ind == INPUT_NUMBER_EOL) {
        if (q != 0) {
          next_line();
          break;
        }
        snprintf(g_error_message, sizeof(g_error_message), "MACRO \"%s\" is missing argument names?\n", m->name);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      q++;

      /* store the label */
      m->argument_names = realloc(m->argument_names, sizeof(char *)*q);
      if (m->argument_names == NULL) {
        print_error("Out of memory error.\n", ERROR_NONE);
        return FAILED;
      }
      m->argument_names[q-1] = calloc(strlen(tmp)+1, 1);
      if (m->argument_names[q-1] == NULL) {
        print_error("Out of memory error.\n", ERROR_NONE);
        return FAILED;
      }

      strcpy(m->argument_names[q-1], tmp);
    }
  }

  m->nargument_names = q;
  m->start = g_source_pointer;
  m->start_line = g_active_file_info_last->line_current;

  /* go to the end of the macro */
  for (; g_source_pointer < g_size; g_source_pointer++) {
    if (g_buffer[g_source_pointer] == 0x0A) {
      next_line();
      continue;
    }
    else if ((strncmp(&g_buffer[g_source_pointer], ".E", 2) == 0) && (g_buffer[g_source_pointer + 2] == 0x0A || g_buffer[g_source_pointer + 2] == ' ')) {
      g_active_file_info_last->line_current = macro_start_line;
      snprintf(g_error_message, sizeof(g_error_message), "MACRO \"%s\" wasn't terminated with .ENDM.\n", m->name);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    else if ((strncmp(&g_buffer[g_source_pointer], ".ENDM", 5) == 0 || strncmp(&g_buffer[g_source_pointer], ".endm", 5) == 0) && (g_buffer[g_source_pointer + 5] == 0x0A || g_buffer[g_source_pointer + 5] == ' ')) {
      g_source_pointer += 5;
      break;
    }
  }

  return SUCCEEDED;
}


int directive_rept_repeat(void) {
  
  char c[16], index_name[MAX_NAME_LENGTH + 1];
  int q;

  strcpy(c, cp);

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs a count.\n", c);
    print_error(g_error_message, ERROR_INP);
    return FAILED;
  }

  if (d < 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s count value must be positive or zero.\n", c);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  index_name[0] = 0;
  if (compare_next_token("INDEX") == SUCCEEDED) {
    skip_next_token();

    ind = input_next_string();
    if (ind != SUCCEEDED)
      return FAILED;

    if (redefine(tmp, 0.0, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
      return FAILED;

    strcpy(index_name, tmp);
  }
    
  if (d == 0) {
    int l, r, m;

    l = g_active_file_info_last->line_current;
    /* find the next compiling point */
    r = 1;
    m = g_macro_active;
    /* disable macro decoding */
    g_macro_active = 0;
    while (get_next_token() != FAILED) {
      if (tmp[0] == '.') {
        if (strcaselesscmp(cp, "ENDR") == 0)
          r--;
        if (strcaselesscmp(cp, "E") == 0)
          break;
        if (strcaselesscmp(cp, "REPT") == 0 || strcaselesscmp(cp, "REPEAT") == 0)
          r++;
      }
      if (r == 0) {
        g_macro_active = m;
        return SUCCEEDED;
      }
    }
    
    /* return the condition's line number */
    g_active_file_info_last->line_current = l;
    snprintf(g_error_message, sizeof(g_error_message), ".%s must end to .ENDR.\n", c);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (g_repeat_active == g_repeat_stack_size) {
    struct repeat_runtime *rr;

    g_repeat_stack_size = (g_repeat_stack_size<<1)+2;
    rr = realloc(g_repeat_stack, sizeof(struct repeat_runtime) * g_repeat_stack_size);
    if (rr == NULL) {
      print_error("Out of memory error while enlarging repeat stack buffer.\n", ERROR_ERR);
      return FAILED;
    }
    g_repeat_stack = rr;
  }

  g_repeat_stack[g_repeat_active].start = g_source_pointer;
  g_repeat_stack[g_repeat_active].counter = d;
  g_repeat_stack[g_repeat_active].repeats = 0;
  g_repeat_stack[g_repeat_active].start_line = g_active_file_info_last->line_current;
  strcpy(g_repeat_stack[g_repeat_active].index_name, index_name);

  g_repeat_active++;

  /* repeat start */
  fprintf(g_file_out_ptr, "j ");
      
  return SUCCEEDED;
}


int directive_endm(void) {

  int q;
  
  if (g_macro_active != 0) {
    g_macro_active--;

    /* macro call end */
    fprintf(g_file_out_ptr, "I%s ", g_macro_stack[g_macro_active].macro->name);
    
    /* free the arguments */
    if (g_macro_stack[g_macro_active].supplied_arguments > 0) {
      for (d = 0; d < g_macro_stack[g_macro_active].supplied_arguments; d++)
        free(g_macro_stack[g_macro_active].argument_data[d]);
      free(g_macro_stack[g_macro_active].argument_data);
      g_macro_stack[g_macro_active].argument_data = NULL;
    }

    /* free the argument definitions */
    for (q = 0; q < g_macro_stack[g_macro_active].macro->nargument_names; q++)
      undefine(g_macro_stack[g_macro_active].macro->argument_names[q]);

    g_source_pointer = g_macro_stack[g_macro_active].macro_return_i;

    if ((g_extra_definitions == ON) && (g_active_file_info_last->filename_id != g_macro_stack[g_macro_active].macro_return_filename_id)) {
      redefine("WLA_FILENAME", 0.0, get_file_name(g_macro_stack[g_macro_active].macro_return_filename_id), DEFINITION_TYPE_STRING,
               (int)strlen(get_file_name(g_macro_stack[g_macro_active].macro_return_filename_id)));
      redefine("wla_filename", 0.0, get_file_name(g_macro_stack[g_macro_active].macro_return_filename_id), DEFINITION_TYPE_STRING,
               (int)strlen(get_file_name(g_macro_stack[g_macro_active].macro_return_filename_id)));
    }

    g_active_file_info_last->filename_id = g_macro_stack[g_macro_active].macro_return_filename_id;
    g_active_file_info_last->line_current = g_macro_stack[g_macro_active].macro_return_line;

    /* was this the last macro called? */
    if (g_macro_active == 0) {
      /* delete NARGS */
      undefine("NARGS");
      undefine("nargs");

      g_macro_runtime_current = NULL;
    }
    else {
      /* redefine NARGS */
      if (redefine("NARGS", (double)g_macro_stack[g_macro_active - 1].supplied_arguments, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
        return FAILED;
      if (redefine("nargs", (double)g_macro_stack[g_macro_active - 1].supplied_arguments, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
        return FAILED;

      g_macro_runtime_current = &g_macro_stack[g_macro_active - 1];
    }

    /* was this a DBM macro call? */
    if (g_macro_stack[g_macro_active].caller == MACRO_CALLER_DBM) {
      /* yep, get the output */
      if (macro_insert_byte_db("DBM") == FAILED)
        return FAILED;

      /* continue defining bytes */
      if (macro_start_dxm(g_macro_stack[g_macro_active].macro, MACRO_CALLER_DBM, "DBM", NO) == FAILED)
        return FAILED;
    }
    /* was this a DWM macro call? */
    else if (g_macro_stack[g_macro_active].caller == MACRO_CALLER_DWM) {
      /* yep, get the output */
      if (macro_insert_word_db("DWM") == FAILED)
        return FAILED;

      /* continue defining words */
      if (macro_start_dxm(g_macro_stack[g_macro_active].macro, MACRO_CALLER_DWM, "DWM", NO) == FAILED)
        return FAILED;
    }
#if W65816
    /* was this a DLM macro call? */
    else if (g_macro_stack[g_macro_active].caller == MACRO_CALLER_DLM) {
      /* yep, get the output */
      if (macro_insert_long_db("DLM") == FAILED)
        return FAILED;

      /* continue defining longs */
      if (macro_start_dxm(g_macro_stack[g_macro_active].macro, MACRO_CALLER_DLM, "DLM", NO) == FAILED)
        return FAILED;
    }
#endif
    /* or was this an INCBIN with a filter macro call? */
    else if (g_macro_stack[g_macro_active].caller == MACRO_CALLER_INCBIN) {
      /* yep, get the output */
      if (macro_insert_byte_db("INCBIN") == FAILED)
        return FAILED;

      /* continue filtering the binary file */
      if (macro_start_incbin(g_macro_stack[g_macro_active].macro, NULL, NO) == FAILED)
        return FAILED;
    }

    return SUCCEEDED;
  }

  snprintf(g_error_message, sizeof(g_error_message), "No .MACRO open.\n");
  print_error(g_error_message, ERROR_DIR);

  return FAILED;
}


#ifdef W65816

int directive_snesheader(void) {

  int q;
  
  if (g_snesheader_defined != 0) {
    print_error(".SNESHEADER can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_computesneschecksum_defined != 0)
    print_error(".COMPUTESNESCHECKSUM is unnecessary when .SNESHEADER is defined.\n", ERROR_WRN);
  else
    g_computesneschecksum_defined++;

  if (g_output_format == OUTPUT_LIBRARY) {
    print_error("Libraries don't take .SNESHEADER.\n", ERROR_DIR);
    return FAILED;
  }

  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDSNES") == 0)
      break;
    else if (strcaselesscmp(tmp, "ID") == 0) {
      if ((ind = get_next_token()) == FAILED)
        return FAILED;

      if (ind != GET_NEXT_TOKEN_STRING || tmp[4] != 0) {
        print_error("ID requires a string of 1 to 4 letters.\n", ERROR_DIR);
        return FAILED;
      }

      /* no ID has been defined so far */
      if (g_snesid_defined == 0) {
        for (ind = 0; tmp[ind] != 0 && ind < 4; ind++)
          g_snesid[ind] = tmp[ind];

        for ( ; ind < 4; g_snesid[ind] = 0, ind++)
          ;

        g_snesid_defined = 1;
      }
      /* compare the IDs */
      else {
        for (ind = 0; tmp[ind] != 0 && g_snesid[ind] != 0 && ind < 4; ind++)
          if (g_snesid[ind] != tmp[ind])
            break;

        if (ind == 4 && tmp[ind] != 0) {
          print_error("ID requires a string of 1 to 4 letters.\n", ERROR_DIR);
          return FAILED;
        }
        if (ind != 4 && (g_snesid[ind] != 0 || tmp[ind] != 0)) {
          print_error("ID was already defined.\n", ERROR_DIR);
          return FAILED;
        }
      }
    }    
    else if (strcaselesscmp(tmp, "NAME") == 0) {
      if ((ind = get_next_token()) == FAILED)
        return FAILED;

      if (ind != GET_NEXT_TOKEN_STRING) {
        print_error("NAME requires a string of 1 to 21 letters.\n", ERROR_DIR);
        return FAILED;
      }

      /* no name has been defined so far */
      if (g_name_defined == 0) {
        for (ind = 0; tmp[ind] != 0 && ind < 21; ind++)
          g_name[ind] = tmp[ind];

        if (ind == 21 && tmp[ind] != 0) {
          print_error("NAME requires a string of 1 to 21 letters.\n", ERROR_DIR);
          return FAILED;
        }

        for ( ; ind < 21; g_name[ind] = 0, ind++)
          ;

        g_name_defined = 1;
      }
      /* compare the names */
      else {
        for (ind = 0; tmp[ind] != 0 && g_name[ind] != 0 && ind < 21; ind++)
          if (g_name[ind] != tmp[ind])
            break;

        if (ind == 21 && tmp[ind] != 0) {
          print_error("NAME requires a string of 1 to 21 letters.\n", ERROR_DIR);
          return FAILED;
        }
        if (ind != 21 && (g_name[ind] != 0 || tmp[ind] != 0)) {
          print_error("NAME was already defined.\n", ERROR_DIR);
          return FAILED;
        }
      }
    }
    else if (strcaselesscmp(tmp, "HIROM") == 0) {
      if (g_lorom_defined != 0 || g_exlorom_defined != 0 || g_exhirom_defined != 0) {
        give_snes_rom_mode_defined_error(".HIROM");
        return FAILED;
      }

      g_hirom_defined++;
    }
    else if (strcaselesscmp(tmp, "EXHIROM") == 0) {
      if (g_lorom_defined != 0 || g_exlorom_defined != 0 || g_hirom_defined != 0) {
        give_snes_rom_mode_defined_error(".EXHIROM");
        return FAILED;
      }

      g_exhirom_defined++;
    }
    else if (strcaselesscmp(tmp, "LOROM") == 0) {
      if (g_hirom_defined != 0 || g_exlorom_defined != 0 || g_exhirom_defined != 0) {
        give_snes_rom_mode_defined_error(".LOROM");
        return FAILED;
      }

      g_lorom_defined++;
    }
    /*
      else if (strcaselesscmp(tmp, "EXLOROM") == 0) {
      if (g_hirom_defined != 0 || g_lorom_defined != 0 || g_exhirom_defined != 0) {
      give_snes_rom_mode_defined_error(".EXLOROM");
      return FAILED;
      }

      g_exlorom_defined++;
      }
    */
    else if (strcaselesscmp(tmp, "SLOWROM") == 0) {
      if (g_fastrom_defined != 0) {
        print_error(".FASTROM was defined prior to .SLOWROM.\n", ERROR_DIR);
        return FAILED;
      }

      g_slowrom_defined++;
    }
    else if (strcaselesscmp(tmp, "FASTROM") == 0) {
      if (g_slowrom_defined != 0) {
        print_error(".SLOWROM was defined prior to .FASTROM.\n", ERROR_DIR);
        return FAILED;
      }

      g_fastrom_defined++;
    }
    else if (strcaselesscmp(tmp, "CARTRIDGETYPE") == 0) {
      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "CARTRIDGETYPE expects 8-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_cartridgetype_defined != 0 && d != g_cartridgetype) {
          print_error("CARTRIDGETYPE was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_cartridgetype = d;
        g_cartridgetype_defined = 1;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "ROMSIZE") == 0) {
      if (g_snesromsize != 0) {
        print_error("ROMSIZE can be defined only once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "ROMSIZE expects 8-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED)
        g_snesromsize = d;
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "SRAMSIZE") == 0) {
      if (g_sramsize_defined != 0) {
        print_error("SRAMSIZE can be defined only once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < 0 || d > 3)) {
        snprintf(g_error_message, sizeof(g_error_message), "SRAMSIZE expects 0-3, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        g_sramsize = d;
        g_sramsize_defined++;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "COUNTRY") == 0) {
      if (g_country_defined != 0) {
        print_error("COUNTRY can be defined only once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "COUNTRY expects 8-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        g_country = d;
        g_country_defined++;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "LICENSEECODE") == 0) {
      if (g_licenseecode_defined != 0) {
        print_error("LICENSEECODE can be defined only once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "LICENSEECODE expects 8-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        g_licenseecode = d;
        g_licenseecode_defined++;
      }
      else
        return FAILED;
    }
    else if (strcaselesscmp(tmp, "VERSION") == 0) {
      inz = input_number();

      if (inz == SUCCEEDED && (d < -128 || d > 255)) {
        snprintf(g_error_message, sizeof(g_error_message), "VERSION expects 8-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      else if (inz == SUCCEEDED) {
        if (g_version_defined != 0 && g_version != d) {
          print_error("VERSION was defined for the second time.\n", ERROR_DIR);
          return FAILED;
        }

        g_version = d;
        g_version_defined++;
      }
      else
        return FAILED;
    }
    else {
      ind = FAILED;
      break; 
    } 
  }

  if (ind != SUCCEEDED) {
    print_error("Error in .SNESHEADER data structure.\n", ERROR_DIR);
    return FAILED;
  }

  g_snesheader_defined = 1;
  g_snes_mode++;

  return SUCCEEDED;
}


int directive_snesnativevector(void) {
  
  int cop_defined = 0, brk_defined = 0, abort_defined = 0, base_address = 0;
  int nmi_defined = 0, unused_defined = 0, irq_defined = 0, q;
  char cop[512], brk[512], abort[512], nmi[512], unused[512], irq[512];

  if (g_snesnativevector_defined != 0) {
    print_error(".SNESNATIVEVECTOR can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_hirom_defined == 0 && g_lorom_defined == 0 && g_exhirom_defined == 0 && g_exlorom_defined == 0) {
    print_error(".SNESNATIVEVECTOR needs .LOROM, .HIROM or .EXHIROM defined earlier.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_output_format == OUTPUT_LIBRARY) {
    print_error("Libraries don't take .SNESNATIVEVECTOR.\n", ERROR_DIR);
    return FAILED;
  }

  /* create a section for the data */
  if (create_a_new_section_structure() == FAILED)
    return FAILED;
  strcpy(g_sec_tmp->name, "!__WLA_SNESNATIVEVECTOR");
  g_sec_tmp->status = SECTION_STATUS_ABSOLUTE;
  
  if (g_lorom_defined || g_exlorom_defined)
    base_address = 0x7FE4;
  else if (g_hirom_defined)
    base_address = 0xFFE4;
  else if (g_exhirom_defined)
    base_address = 0x40FFE4;
    
  fprintf(g_file_out_ptr, "P O0 A%d %d ", g_sec_tmp->id, base_address);
  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDNATIVEVECTOR") == 0) {
      if (cop_defined == 0)
        snprintf(cop, sizeof(cop), "y%d ", 0x0000);
      if (brk_defined == 0)
        snprintf(brk, sizeof(brk), "y%d ", 0x0000);
      if (abort_defined == 0)
        snprintf(abort, sizeof(abort), "y%d ", 0x0000);
      if (nmi_defined == 0)
        snprintf(nmi, sizeof(nmi), "y%d ", 0x0000);
      if (unused_defined == 0) 
        snprintf(unused, sizeof(unused), "y%d ", 0x0000);
      if (irq_defined == 0)
        snprintf(irq, sizeof(irq), "y%d ", 0x0000);
      
      fprintf(g_file_out_ptr, "%s%s%s%s%s%s s p ", cop, brk, abort, nmi, unused, irq);
      
      break;
    }
    else if (strcaselesscmp(tmp, "COP") == 0) {
      if (cop_defined != 0) {
        print_error("COP can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "COP expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(cop, sizeof(cop), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(cop, sizeof(cop), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(cop, sizeof(cop), "C%d ", g_latest_stack);

      cop_defined++;
    }
    else if (strcaselesscmp(tmp, "BRK") == 0) {
      if (brk_defined != 0) {
        print_error("BRK can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "BRK expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(brk, sizeof(brk), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(brk, sizeof(brk), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(brk, sizeof(brk), "C%d ", g_latest_stack);

      brk_defined++;
    }
    else if (strcaselesscmp(tmp, "ABORT") == 0) {
      if (abort_defined != 0) {
        print_error("ABORT can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "ABORT expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(abort, sizeof(abort), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(abort, sizeof(abort), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(abort, sizeof(abort), "C%d ", g_latest_stack);

      abort_defined++;
    }
    else if (strcaselesscmp(tmp, "NMI") == 0) {
      if (nmi_defined != 0) {
        print_error("NMI can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "NMI expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(nmi, sizeof(nmi), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(nmi, sizeof(nmi), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(nmi, sizeof(nmi), "C%d ", g_latest_stack);

      nmi_defined++;
    }
    else if (strcaselesscmp(tmp, "UNUSED") == 0) {
      if (unused_defined != 0) {
        print_error("UNUSED can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "UNUSED expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(unused, sizeof(unused), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(unused, sizeof(unused), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(unused, sizeof(unused), "C%d ", g_latest_stack);

      unused_defined++;
    }
    else if (strcaselesscmp(tmp, "IRQ") == 0) {
      if (irq_defined != 0) {
        print_error("IRQ can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "IRQ expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(irq, sizeof(irq), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(irq, sizeof(irq), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(irq, sizeof(irq), "C%d ", g_latest_stack);

      irq_defined++;
    }
    else {
      ind = FAILED;
      break;
    }
  }

  if (ind != SUCCEEDED) {
    print_error("Error in .SNESNATIVEVECTOR data structure.\n", ERROR_DIR);
    return FAILED;
  }

  g_snesnativevector_defined = 1;
  g_snes_mode++;

  return SUCCEEDED;
}


int directive_snesemuvector(void) {
  
  int cop_defined = 0, unused_defined = 0, abort_defined = 0, base_address = 0;
  int nmi_defined = 0, reset_defined = 0, irqbrk_defined = 0, q;
  char cop[512], unused[512], abort[512], nmi[512], reset[512], irqbrk[512];

  if (g_snesemuvector_defined != 0) {
    print_error(".SNESEMUVECTOR can be defined only once.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_hirom_defined == 0 && g_lorom_defined == 0 && g_exhirom_defined == 0 && g_exlorom_defined == 0) {
    print_error(".SNESEMUVECTOR needs .LOROM, .HIROM or .EXHIROM defined earlier.\n", ERROR_DIR);
    return FAILED;
  }
  if (g_output_format == OUTPUT_LIBRARY) {
    print_error("Libraries don't take .SNESEMUVECTOR.\n", ERROR_DIR);
    return FAILED;
  }

  /* create a section for the data */
  if (create_a_new_section_structure() == FAILED)
    return FAILED;
  strcpy(g_sec_tmp->name, "!__WLA_SNESEMUVECTOR");
  g_sec_tmp->status = SECTION_STATUS_ABSOLUTE;

  if (g_lorom_defined || g_exlorom_defined)
    base_address = 0x7FF4;
  else if (g_hirom_defined)
    base_address = 0xFFF4;
  else if (g_exhirom_defined)
    base_address = 0x40FFF4;

  fprintf(g_file_out_ptr, "P O0 A%d %d ", g_sec_tmp->id, base_address);
  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  while ((ind = get_next_token()) == SUCCEEDED) {
    /* .IF directive? */
    if (tmp[0] == '.') {
      q = parse_if_directive();
      if (q == FAILED)
        return FAILED;
      else if (q == SUCCEEDED)
        continue;
      /* else q == -1, continue executing below */
    }

    if (strcaselesscmp(tmp, ".ENDEMUVECTOR") == 0) {
      if (cop_defined == 0)
        snprintf(cop, sizeof(cop), "y%d ", 0);
      if (reset_defined == 0)
        snprintf(reset, sizeof(reset), "y%d ", 0);
      if (abort_defined == 0)
        snprintf(abort, sizeof(abort), "y%d ", 0);
      if (nmi_defined == 0)
        snprintf(nmi, sizeof(nmi), "y%d ", 0);
      if (unused_defined == 0)
        snprintf(unused, sizeof(unused), "y%d ", 0);
      if (irqbrk_defined == 0)
        snprintf(irqbrk, sizeof(irqbrk), "y%d ", 0);

      fprintf(g_file_out_ptr, "%s%s%s%s%s%s s p ", cop, unused, abort, nmi, reset, irqbrk);
      
      break;
    }
    else if (strcaselesscmp(tmp, "COP") == 0) {
      if (cop_defined != 0) {
        print_error("COP can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "COP expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(cop, sizeof(cop), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(cop, sizeof(cop), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(cop, sizeof(cop), "C%d ", g_latest_stack);

      cop_defined++;
    }
    else if (strcaselesscmp(tmp, "RESET") == 0) {
      if (reset_defined != 0) {
        print_error("RESET can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "RESET expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(reset, sizeof(reset), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(reset, sizeof(reset), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(reset, sizeof(reset), "C%d ", g_latest_stack);

      reset_defined++;
    }
    else if (strcaselesscmp(tmp, "ABORT") == 0) {
      if (abort_defined != 0) {
        print_error("ABORT can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "ABORT expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(abort, sizeof(abort), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(abort, sizeof(abort), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(abort, sizeof(abort), "C%d ", g_latest_stack);

      abort_defined++;
    }
    else if (strcaselesscmp(tmp, "NMI") == 0) {
      if (nmi_defined != 0) {
        print_error("NMI can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "NMI expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(nmi, sizeof(nmi), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(nmi, sizeof(nmi), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(nmi, sizeof(nmi), "C%d ", g_latest_stack);

      nmi_defined++;
    }
    else if (strcaselesscmp(tmp, "UNUSED") == 0) {
      if (unused_defined != 0) {
        print_error("UNUSED can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();

      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "UNUSED expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(unused, sizeof(unused), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(unused, sizeof(unused), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(unused, sizeof(unused), "C%d ", g_latest_stack);

      unused_defined++;
    }
    else if (strcaselesscmp(tmp, "IRQBRK") == 0) {
      if (irqbrk_defined != 0) {
        print_error("IRQBRK can only be defined once.\n", ERROR_DIR);
        return FAILED;
      }

      inz = input_number();
      
      if (inz == SUCCEEDED && (d < -32768 || d > 65535)) {
        snprintf(g_error_message, sizeof(g_error_message), "IRQBRK expects 16-bit data, %d is out of range!\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      if (inz == SUCCEEDED)
        snprintf(irqbrk, sizeof(irqbrk), "y%d ", d);
      else if (inz == INPUT_NUMBER_ADDRESS_LABEL)
        snprintf(irqbrk, sizeof(irqbrk), "k%d r%s ", g_active_file_info_last->line_current, g_label);
      else if (inz == INPUT_NUMBER_STACK)
        snprintf(irqbrk, sizeof(irqbrk), "C%d ", g_latest_stack);

      irqbrk_defined++;
    }
    else {
      ind = FAILED;
      break;
    }
  }
  
  if (ind != SUCCEEDED) {
    print_error("Error in .SNESEMUVECTOR data structure.\n", ERROR_DIR);
    return FAILED;
  }

  g_snesemuvector_defined = 1;
  g_snes_mode++;

  return SUCCEEDED;
}

#endif


int directive_print(void) {

  int get_value, value_type;

  while (1) {
    get_value = NO;
    value_type = 1;
      
    if (compare_next_token("HEX") == SUCCEEDED) {
      if (skip_next_token() == FAILED)
        return FAILED;
      
      value_type = 0;
      get_value = YES;
    }
    else if (compare_next_token("DEC") == SUCCEEDED) {
      if (skip_next_token() == FAILED)
        return FAILED;

      value_type = 1;
      get_value = YES;
    }

    inz = input_number();

    if (inz == INPUT_NUMBER_STRING || inz == INPUT_NUMBER_ADDRESS_LABEL) {
      char t[256];
    
      if (get_value == YES) {
        print_error(".PRINT was expecting a value, got a string/label instead.\n", ERROR_INP);
        return FAILED;
      }

      parse_print_string(g_label, t, 256);

      if (g_quiet == NO) {
        printf("%s", t);
        fflush(stdout);
      }
    }
    else if (inz == SUCCEEDED) {
      if (g_quiet == NO) {
        if (value_type == 0)
          printf("%x", d);
        else
          printf("%d", d);
        fflush(stdout);
      }
    }
    else if (inz == INPUT_NUMBER_EOL) {
      next_line();
      break;
    }
    else {
      print_error(".PRINT needs a string/label or (an optional) HEX/DEC plus a value.\n", ERROR_DIR);
      return FAILED;
    }
  }

  return SUCCEEDED;
}


int directive_printt(void) {
  
  char t[256];
    
  inz = input_number();

  if (inz != INPUT_NUMBER_STRING && inz != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".PRINTT needs a string/label.\n", ERROR_DIR);
    return FAILED;
  }

  parse_print_string(g_label, t, 256);
    
  if (g_quiet == NO) {
    printf("%s", t);
    fflush(stdout);
  }

  return SUCCEEDED;
}


int directive_printv(void) {

  int m = 1, q;

  if (compare_next_token("HEX") == SUCCEEDED) {
    skip_next_token();
    m = 0;
  }
  else if (compare_next_token("DEC") == SUCCEEDED) {
    skip_next_token();
    m = 1;
  }

  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    if (q == INPUT_NUMBER_ADDRESS_LABEL) {
      snprintf(g_error_message, sizeof(g_error_message), "\"%s\" is not known.\n", g_label);
      print_error(g_error_message, ERROR_DIR);
    }
    print_error(".PRINTV can only print currently known values.\n", ERROR_DIR);
    return FAILED;
  }

  if (g_quiet == NO) {
    if (m == 0)
      printf("%x", d);
    else
      printf("%d", d);
    fflush(stdout);
  }

  return SUCCEEDED;
}


int directive_dbrnd_dwrnd(void) {
  
  int o, c, min, max, f, q;

  /* bytes or words? */
  if (cp[1] == 'W')
    o = 1;
  else
    o = 0;

  /* get the count */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs the number of random numbers.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  c = d;

  if (c <= 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs that the number of random numbers is > 0.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* get min */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs the min value.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  min = d;

  /* get max */
  q = input_number();
  if (q == FAILED)
    return FAILED;
  if (q != SUCCEEDED) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs the max value.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  max = d;

  if (min >= max) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs that min < max.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  /* generate the numbers */
  for (f = 0; f < c; f++) {
    d = (genrand_int32() % (max-min+1)) + min;
    
    if (o == 1) {
      if (d < -32768 || d > 65535) {
        snprintf(g_error_message, sizeof(g_error_message), ".%s: Expected a 16-bit value, computed %d.\n", cp, d);
        print_error(g_error_message, ERROR_NONE);
        return FAILED;
      }
      fprintf(g_file_out_ptr, "y %d ", d);
    }
    else {
      if (d > 255 || d < -128) {
        snprintf(g_error_message, sizeof(g_error_message), ".%s: Expected a 8-bit value, computed %d.\n", cp, d);
        print_error(g_error_message, ERROR_NONE);
        return FAILED;
      }
      fprintf(g_file_out_ptr, "d%d ", d);
    }
  }

  return SUCCEEDED;
}


int directive_dwsin_dbsin_dwcos_dbcos(void) {
  
  double m, a, s, n;
  int p, c, o, f;

  if (cp[1] == 'W')
    o = 1;
  else
    o = 2;
  if (cp[2] == 'S')
    f = 1;
  else
    f = 2;

  g_input_float_mode = ON;
  p = input_number();
  g_input_float_mode = OFF;
  if (p != SUCCEEDED && p != INPUT_NUMBER_FLOAT) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs a value for starting angle.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (p == SUCCEEDED)
    a = d;
  else
    a = g_parsed_double;

  if (input_number() != SUCCEEDED || d < 0) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs an non-negative integer value for additional angles.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  c = d;

  g_input_float_mode = ON;
  p = input_number();
  if (p != SUCCEEDED && p != INPUT_NUMBER_FLOAT) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs a value for angle step.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (p == SUCCEEDED)
    s = d;
  else
    s = g_parsed_double;

  p = input_number();
  if (p != SUCCEEDED && p != INPUT_NUMBER_FLOAT) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs a value to multiply the result with.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (p == SUCCEEDED)
    m = d;
  else
    m = g_parsed_double;

  p = input_number();
  g_input_float_mode = OFF;
  if (p != SUCCEEDED && p != INPUT_NUMBER_FLOAT) {
    snprintf(g_error_message, sizeof(g_error_message), ".%s needs a value to add to the result.\n", cp);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  if (p == SUCCEEDED)
    n = d;
  else
    n = g_parsed_double;

  for (c++; c > 0; c--) {
    while (a >= 360)
      a -= 360;

    if (f == 1)
      d = (int)((m * sin(a*M_PI/180)) + n);
    else
      d = (int)((m * cos(a*M_PI/180)) + n);

    if (o == 1) {
      if (d < -32768 || d > 65535) {
        snprintf(g_error_message, sizeof(g_error_message), ".%s: Expected a 16-bit value, computed %d.\n", cp, d);
        print_error(g_error_message, ERROR_NONE);
        return FAILED;
      }
      fprintf(g_file_out_ptr, "y %d ", d);
    }
    else {
      if (d > 255 || d < -128) {
        snprintf(g_error_message, sizeof(g_error_message), ".%s: Expected a 8-bit value, computed %d.\n", cp, d);
        print_error(g_error_message, ERROR_NONE);
        return FAILED;
      }
      fprintf(g_file_out_ptr, "d%d ", d);
    }

    a += s;
  }

  return SUCCEEDED;
}


int directive_stringmap_table(void) {

  int parse_result, line_number = 0;
  FILE* table_file;
  char line_buffer[256];
  struct stringmaptable *map;

  g_expect_calculations = NO;
  parse_result = input_number();
  g_expect_calculations = YES;

  if (parse_result != INPUT_NUMBER_STRING && parse_result != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".STRINGMAPTABLE needs a file name string.\n", ERROR_DIR);
    return FAILED;
  }

  /* Allocate and insert at the front of the chain */
  map = calloc(sizeof(struct stringmaptable), 1);
  if (map == NULL) {
    print_error("STRINGMAPTABLE: Out of memory error.\n", ERROR_ERR);
    return FAILED;
  }
  map->next = g_stringmaptables;
  g_stringmaptables = map;

  strcpy(map->name, g_label);

  g_expect_calculations = NO;
  parse_result = input_number();
  g_expect_calculations = YES;

  if (parse_result != INPUT_NUMBER_STRING && parse_result != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".STRINGMAPTABLE needs a file name string.\n", ERROR_DIR);
    return FAILED;
  }

  table_file = fopen(g_label, "r");
  if (table_file == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Error opening file \"%s\".\n", g_label);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  while (fgets(line_buffer, 256, table_file)) {
    char* p = line_buffer, *equals_pos;
    struct stringmap_entry* entry;
    int char_count;
    unsigned char* bytes_writer;
    int accumulator = 0;

    line_number++;

    /* skip comments */
    if (*p == ';' || *p == '#')
      continue;

    equals_pos = strchr(p, '=');

    /* lines should be in the form <hex>=<text> with no whitespace. */
    if (equals_pos == NULL)
      continue;

    entry = calloc(sizeof(struct stringmap_entry), 1);
    if (entry == NULL) {
      print_error("STRINGMAPTABLE: Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }

    /* insert at front of entries list */
    entry->next = map->entries;
    map->entries = entry;

    /* left of = should be a string of hex digits, for a variable whole number of bytes */
    char_count = (int)(equals_pos - p);
    if (char_count == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "STRINGMAPTABLE: No text before '=' at line %d of file \"%s\".\n", line_number, g_label);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    entry->bytes_length = char_count / 2 + char_count % 2;
    entry->bytes = calloc(sizeof(unsigned char), entry->bytes_length);
    if (entry->bytes == NULL) {
      print_error("STRINGMAPTABLE: Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }
    bytes_writer = entry->bytes;
    for (; p != equals_pos; ++p) {
      /* parse character as hex */
      const char c = *p;
      if (c >= '0' && c <= '9')
        accumulator |= c - '0';
      else if (c >= 'a' && c <= 'f')
        accumulator |= c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        accumulator |= c - 'A' + 10;
      else {
        snprintf(g_error_message, sizeof(g_error_message), "STRINGMAPTABLE: Invalid hex character '%c' at line %d of file \"%s\".\n", c, line_number, g_label);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }
      /* emit to buffer or shift depending on position */
      if ((equals_pos - p) % 2 == 0) {
        /* even count -> shift */
        accumulator <<= 4;
      }
      else {
        /* odd -> finished a byte */
        *bytes_writer++ = (unsigned char)accumulator;
        accumulator = 0;
      }
    }
    /* then the string. we want to remove any trailing CRLF. */
    p[strcspn(p, "\r\n")] = 0;
    entry->text_length = (int)strlen(++p);
    if (entry->text_length == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "STRINGMAPTABLE: no text after '=' at line %d of file \"%s\".\n", line_number, g_label);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    p = equals_pos + 1;
    entry->text = calloc(sizeof(char), strlen(p) + 1);
    if (entry->text == NULL) {
      print_error("STRINGMAPTABLE: Out of memory error.\n", ERROR_DIR);
      return FAILED;
    }
    strcpy(entry->text, p);
  }

  fclose(table_file);

  return SUCCEEDED;
}


int directive_stringmap(void) {

  int parse_result;
  struct stringmaptable *table;
  char *p;

  /* first get the map name */
  g_expect_calculations = NO;
  parse_result = input_number();
  g_expect_calculations = YES;

  if (parse_result != INPUT_NUMBER_STRING && parse_result != INPUT_NUMBER_ADDRESS_LABEL) {
    print_error(".STRINGMAP needs a table name.\n", ERROR_DIR);
    return FAILED;
  }

  /* find the table */
  for (table = g_stringmaptables; table != NULL; table = table->next) {
    if (strcaselesscmp(table->name, g_label) == 0) {
      /* found it */
      break;
    }
  }
  if (table == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "STRINGMAP: could not find table called \"%s\".\n", g_label);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;    
  }

  /* parse the string */
  parse_result = input_number();
  if (parse_result != INPUT_NUMBER_STRING) {
    print_error("STRINGMAP: no string given", ERROR_DIR);
    return FAILED;    
  }

  fprintf(g_file_out_ptr, "k%d ", g_active_file_info_last->line_current);

  /* parse it */
  for (p = g_label; *p != 0; /* increment in loop */) {
    struct stringmap_entry *candidate, *entry = NULL;
    int i;

    /* find the longest match for the current string position */
    for (candidate = table->entries; candidate != NULL; candidate = candidate->next) {
      /* skip candidates not longer than the current best */
      if (entry != NULL && entry->text_length >= candidate->text_length)
        continue;

      /* check for a match */
      if (strncmp(p, candidate->text, candidate->text_length) == 0)
        entry = candidate;
    }
    /* if no match was found, it's an error */
    if (entry == NULL) {
      snprintf(g_error_message, sizeof(g_error_message), "STRINGMAP: could not find a match in the table at substring \"%s\".\n", p);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;    
    }
    /* else emit */
    for (i = 0; i < entry->bytes_length; ++i)
      fprintf(g_file_out_ptr, "d%d ", entry->bytes[i]);

    /* move pointer on by as much as was matched */
    p += entry->text_length;
  }

  return SUCCEEDED;
}


int parse_directive(void) {

  int q;

  if ((q = parse_if_directive()) != -1)
    return q;
  
  /* ORG */

  if (strcaselesscmp(cp, "ORG") == 0)
    return directive_org();

  /* ORGA */

  if (strcaselesscmp(cp, "ORGA") == 0)
    return directive_orga();

  /* SLOT */

  if (strcaselesscmp(cp, "SLOT") == 0)
    return directive_slot();

  /* BANK */

  if (strcaselesscmp(cp, "BANK") == 0)
    return directive_bank();

  /* DBM/DWM? */

  if (strcaselesscmp(cp, "DBM") == 0 || strcaselesscmp(cp, "DWM") == 0)
    return directive_dbm_dwm_dlm();

  /* TABLE? */

  if (strcaselesscmp(cp, "TABLE") == 0)
    return directive_table();
    
  /* ROW/DATA? */

  if (strcaselesscmp(cp, "ROW") == 0 || strcaselesscmp(cp, "DATA") == 0)
    return directive_row_data();

  /* DB/BYT/BYTE? */

  if (strcaselesscmp(cp, "DB") == 0 || strcaselesscmp(cp, "BYT") == 0 || strcaselesscmp(cp, "BYTE") == 0)
    return directive_db_byt_byte();

  /* ASCTABLE/ASCIITABLE? */

  if (strcaselesscmp(cp, "ASCTABLE") == 0 || strcaselesscmp(cp, "ASCIITABLE") == 0)
    return directive_asctable_asciitable();

  /* ASC? */

  if (strcaselesscmp(cp, "ASC") == 0)
    return directive_asc();

  /* DW/WORD/ADDR? */

  if (strcaselesscmp(cp, "DW") == 0 || strcaselesscmp(cp, "WORD") == 0 || strcaselesscmp(cp, "ADDR") == 0)
    return directive_dw_word_addr();

#ifdef W65816
  
  /* DLM? */

  if (strcaselesscmp(cp, "DLM") == 0)
    return directive_dbm_dwm_dlm();

  /* DL/LONG/FARADDR? */

  if (strcaselesscmp(cp, "DL") == 0 || strcaselesscmp(cp, "LONG") == 0 || strcaselesscmp(cp, "FARADDR") == 0)
    return directive_dl_long_faraddr();

  /* DSL? */

  if (strcaselesscmp(cp, "DSL") == 0)
    return directive_dsl();

  /* NAME */

  if (strcaselesscmp(cp, "NAME") == 0)
    return directive_name_w65816();

  /* WDC */

  if (strcaselesscmp(cp, "WDC") == 0) {
    g_use_wdc_standard = 1;
    return SUCCEEDED;
  }

  /* NOWDC */

  if (strcaselesscmp(cp, "NOWDC") == 0) {
    g_use_wdc_standard = 0;
    return SUCCEEDED;
  }
  
#endif

  /* DSTRUCT */

  if (strcaselesscmp(cp, "DSTRUCT") == 0)
    return directive_dstruct();

  /* DS/DSB? */

  if (strcaselesscmp(cp, "DSB") == 0 || strcaselesscmp(cp, "DS") == 0)
    return directive_dsb_ds();

  /* DSW? */

  if (strcaselesscmp(cp, "DSW") == 0)
    return directive_dsw();
    
  /* INCDIR */

  if (strcaselesscmp(cp, "INCDIR") == 0)
    return directive_incdir();

  /* INCLUDE/INC */

  if (strcaselesscmp(cp, "INCLUDE") == 0 || strcaselesscmp(cp, "INC") == 0)
    return directive_include(YES);

  /* INDLUDE/IND (INTERNAL) */

  if (strcaselesscmp(cp, "INDLUDE") == 0 || strcaselesscmp(cp, "IND") == 0)
    return directive_include(NO);

  /* INCBIN */

  if (strcaselesscmp(cp, "INCBIN") == 0)
    return directive_incbin();

  /* OUTNAME */

  if (strcaselesscmp(cp, "OUTNAME") == 0) {
    g_expect_calculations = NO;
    inz = input_number();
    g_expect_calculations = YES;

    if (inz != INPUT_NUMBER_STRING && inz != INPUT_NUMBER_ADDRESS_LABEL) {
      print_error(".OUTNAME needs a file name string.\n", ERROR_DIR);
      return FAILED;
    }

    strcpy(g_final_name, g_label);

    return SUCCEEDED;
  }

  /* STRUCT */

  if (strcaselesscmp(cp, "STRUCT") == 0)
    return directive_struct();

  /* RAMSECTION */

  if (strcaselesscmp(cp, "RAMSECTION") == 0)
    return directive_ramsection();

  /* SECTION */

  if (strcaselesscmp(cp, "SECTION") == 0)
    return directive_section();

  /* FOPEN */

  if (strcaselesscmp(cp, "FOPEN") == 0)
    return directive_fopen();

  /* FCLOSE */

  if (strcaselesscmp(cp, "FCLOSE") == 0)
    return directive_fclose();

  /* FSIZE */

  if (strcaselesscmp(cp, "FSIZE") == 0)
    return directive_fsize();

  /* FREAD */

  if (strcaselesscmp(cp, "FREAD") == 0)
    return directive_fread();

  /* BLOCK */

  if (strcaselesscmp(cp, "BLOCK") == 0)
    return directive_block();

  /* ENDB */

  if (strcaselesscmp(cp, "ENDB") == 0) {
    if (g_block_status <= 0) {
      print_error("There is no open .BLOCK.\n", ERROR_DIR);
      return FAILED;
    }

    g_block_status--;
    fprintf(g_file_out_ptr, "G ");

    return SUCCEEDED;
  }

  /* SHIFT */

  if (strcaselesscmp(cp, "SHIFT") == 0)
    return directive_shift();

  /* ENDS */

  if (strcaselesscmp(cp, "ENDS") == 0) {
    if (g_section_status == OFF) {
      print_error("There is no open section.\n", ERROR_DIR);
      return FAILED;
    }
    if (g_dstruct_status == ON) {
      print_error("You can't close a section inside .DSTRUCT.\n", ERROR_DIR);
      return FAILED;
    }

    /* generate a section end label? */
    if (g_extra_definitions == ON)
      generate_label("SECTIONEND_", g_sections_last->name);
  
    g_section_status = OFF;
    g_bankheader_status = OFF;
    in_ramsection = NO;
    
    fprintf(g_file_out_ptr, "s ");

    return SUCCEEDED;
  }

  /* ROMBANKS */

  if (strcaselesscmp(cp, "ROMBANKS") == 0)
    return directive_rombanks();

  /* ROMBANKMAP */

  if (strcaselesscmp(cp, "ROMBANKMAP") == 0)
    return directive_rombankmap();

  /* MEMORYMAP */

  if (strcaselesscmp(cp, "MEMORYMAP") == 0)
    return directive_memorymap();

  /* UNBACKGROUND */

  if (strcaselesscmp(cp, "UNBACKGROUND") == 0)
    return directive_unbackground();

  /* BACKGROUND */

  if (strcaselesscmp(cp, "BACKGROUND") == 0)
    return directive_background();

#ifdef GB

  /* NINTENDOLOGO */
  
  if (strcaselesscmp(cp, "NINTENDOLOGO") == 0) {
    no_library_files(".NINTENDOLOGO");

    g_nintendologo_defined++;

    return SUCCEEDED;
  }
  
  /* NAME */

  if (strcaselesscmp(cp, "NAME") == 0)
    return directive_name_gb();

  /* RAMSIZE */

  if (strcaselesscmp(cp, "RAMSIZE") == 0) {
    no_library_files(".RAMSIZE");

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || q < 0) {
      print_error(".RAMSIZE needs a non-negative value.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_rambanks_defined != 0) {
      if (g_rambanks != d) {
        print_error(".RAMSIZE was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
    }

    if (d != 0 && d != 1 && d != 2 && d != 3 && d != 4 && d != 5) {
      print_error("Unsupported RAM size.\n", ERROR_DIR);
      return FAILED;
    }

    g_rambanks = d;
    g_rambanks_defined = 1;

    return SUCCEEDED;
  }

  /* COUNTRYCODE */

  if (strcaselesscmp(cp, "COUNTRYCODE") == 0) {
    no_library_files(".COUNTRYCODE");

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || q < 0) {
      print_error(".COUNTRYCODE needs a non-negative value.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_countrycode_defined != 0) {
      if (g_countrycode != d) {
        print_error(".COUNTRYCODE was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
    }

    g_countrycode = d;
    g_countrycode_defined = 1;

    return SUCCEEDED;
  }
    
  /* DESTINATIONCODE */

  if (strcaselesscmp(cp, "DESTINATIONCODE") == 0) {
    no_library_files(".DESTINATIONCODE");

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || q < 0) {
      print_error(".DESTINATIONCODE needs a non-negative value.\n", ERROR_DIR);
      return FAILED;
    }

    if (q == SUCCEEDED && g_countrycode_defined != 0) {
      if (g_countrycode != d) {
        print_error(".DESTINATIONCODE was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
    }

    g_countrycode = d;
    g_countrycode_defined = 1;

    return SUCCEEDED;
  }

  /* CARTRIDGETYPE */

  if (strcaselesscmp(cp, "CARTRIDGETYPE") == 0) {
    no_library_files(".CARTRIDGETYPE");

    q = input_number();

    if (q == SUCCEEDED && g_cartridgetype_defined != 0) {
      if (g_cartridgetype != d) {
        print_error(".CARTRIDGETYPE was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
    }

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error(".CARTRIDGETYPE needs an immediate value.\n", ERROR_DIR);
      return FAILED;
    }

    g_cartridgetype = d;
    g_cartridgetype_defined = 1;

    return SUCCEEDED;
  }

  /* LICENSEECODENEW */

  if (strcaselesscmp(cp, "LICENSEECODENEW") == 0) {
    no_library_files(".LICENSEECODENEW");
    
    if (g_licenseecodeold_defined != 0) {
      print_error(".LICENSEECODENEW and .LICENSEECODEOLD cannot both be defined.\n", ERROR_DIR);
      return FAILED;
    }

    if ((ind = get_next_token()) == FAILED)
      return FAILED;

    if (ind != GET_NEXT_TOKEN_STRING) {
      print_error(".LICENSEECODENEW requires a string of two letters.\n", ERROR_DIR);
      return FAILED;
    }
    if (!(tmp[0] != 0 && tmp[1] != 0 && tmp[2] == 0)) {
      print_error(".LICENSEECODENEW requires a string of two letters.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_licenseecodenew_defined != 0) {
      if (tmp[0] != g_licenseecodenew_c1 || tmp[1] != g_licenseecodenew_c2) {
        print_error(".LICENSEECODENEW was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
      return SUCCEEDED;
    }

    g_licenseecodenew_c1 = tmp[0];
    g_licenseecodenew_c2 = tmp[1];
    g_licenseecodenew_defined = 1;

    return SUCCEEDED;
  }

  /* LICENSEECODEOLD */

  if (strcaselesscmp(cp, "LICENSEECODEOLD") == 0) {
    no_library_files(".LICENSEECODEOLD");
    
    if (g_licenseecodenew_defined != 0) {
      print_error(".LICENSEECODENEW and .LICENSEECODEOLD cannot both be defined.\n", ERROR_DIR);
      return FAILED;
    }

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || d < -128 || d > 255) {
      snprintf(g_error_message, sizeof(g_error_message), ".LICENSEECODEOLD needs a 8-bit value, got %d.\n", d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (g_licenseecodeold_defined != 0) {
      if (g_licenseecodeold != d) {
        print_error(".LICENSEECODEOLD was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
      return SUCCEEDED;
    }

    g_licenseecodeold = d;
    g_licenseecodeold_defined = 1;

    return SUCCEEDED;
  }

  /* VERSION */

  if (strcaselesscmp(cp, "VERSION") == 0) {
    no_library_files(".VERSION");
    
    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || d < -128 || d > 255) {
      snprintf(g_error_message, sizeof(g_error_message), ".VERSION needs a 8-bit value, got %d.\n", d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (g_version_defined != 0) {
      if (g_version != d) {
        print_error(".VERSION was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
      return SUCCEEDED;
    }

    g_version = d;
    g_version_defined = 1;

    return SUCCEEDED;
  }

  /* GBHEADER */

  if (strcmp(cp, "GBHEADER") == 0)
    return directive_gbheader();

#endif

  /* EMPTYFILL */

  if (strcaselesscmp(cp, "EMPTYFILL") == 0) {
    no_library_files(".EMPTYFILL");

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || d < -128 || d > 255) {
      snprintf(g_error_message, sizeof(g_error_message), ".EMPTYFILL needs a 8-bit value, got %d.\n", d);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (g_emptyfill_defined != 0) {
      if (g_emptyfill != d) {
        print_error(".EMPTYFILL was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
      return SUCCEEDED;
    }

    g_emptyfill = d;
    g_emptyfill_defined = 1;

    return SUCCEEDED;
  }

  /* DEFINE/DEF/EQU */

  if (strcaselesscmp(cp, "DEFINE") == 0 || strcaselesscmp(cp, "DEF") == 0 || strcaselesscmp(cp, "EQU") == 0)
    return directive_define_def_equ();

  /* ENUMID */

  if (strcaselesscmp(cp, "ENUMID") == 0)
    return directive_enumid();

  /* INPUT */

  if (strcaselesscmp(cp, "INPUT") == 0)
    return directive_input();

  /* REDEFINE/REDEF */

  if (strcaselesscmp(cp, "REDEFINE") == 0 || strcaselesscmp(cp, "REDEF") == 0)
    return directive_redefine_redef();

  /* EXPORT */

  if (strcaselesscmp(cp, "EXPORT") == 0) {
    q = 0;
    while (1) {
      ind = input_next_string();
      if (ind == FAILED)
        return FAILED;
      if (ind == INPUT_NUMBER_EOL) {
        if (q != 0) {
          next_line();
          return SUCCEEDED;
        }
        print_error(".EXPORT requires definition name(s).\n", ERROR_DIR);
        return FAILED;
      }

      q++;

      if (export_a_definition(tmp) == FAILED)
        return FAILED;
    }

    return FAILED;
  }

  /* SYM/SYMBOL */

  if (strcaselesscmp(cp, "SYM") == 0 || strcaselesscmp(cp, "SYMBOL") == 0) {
    ind = input_next_string();
    if (ind != SUCCEEDED) {
      print_error(".SYM requires a symbol name.\n", ERROR_DIR);
      return FAILED;
    }

    fprintf(g_file_out_ptr, "Y%s ", tmp);

    return SUCCEEDED;
  }

  /* BR/BREAKPOINT */

  if (strcaselesscmp(cp, "BR") == 0 || strcaselesscmp(cp, "BREAKPOINT") == 0) {
    fprintf(g_file_out_ptr, "Z ");
    return SUCCEEDED;
  }

  /* ENUM */

  if (strcaselesscmp(cp, "ENUM") == 0) {
    if (g_dstruct_status == ON) {
      print_error("You can't use start an ENUM inside .DSTRUCT.\n", ERROR_DIR);
      return FAILED;
    }

    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error(".ENUM needs a starting value.\n", ERROR_DIR);
      return FAILED;
    }

    enum_offset = 0;
    last_enum_offset = 0;
    max_enum_offset = 0;
    base_enum_offset = d;

    /* "ASC" or "DESC"? */
    if (compare_next_token("ASC") == SUCCEEDED) {
      enum_ord = 1;
      skip_next_token();
    }
    else if (compare_next_token("DESC") == SUCCEEDED) {
      enum_ord = -1;
      skip_next_token();
    }
    else
      enum_ord = 1;

    /* do we have "EXPORT" defined? */
    if (compare_next_token("EXPORT") == SUCCEEDED) {
      skip_next_token();
      enum_exp = YES;
    }
    else
      enum_exp = NO;

    /* setup active_struct (enum vars stored here temporarily) */
    active_struct = calloc(sizeof(struct structure), 1);
    if (active_struct == NULL) {
      print_error("Out of memory while parsing .ENUM.\n", ERROR_DIR);
      return FAILED;
    }
    active_struct->name[0] = '\0';
    active_struct->items = NULL;
    active_struct->last_item = NULL;
    union_stack = NULL;

    in_enum = YES;

    return SUCCEEDED;
  }

#ifdef GB

  /* COMPUTEGBCHECKSUM */

  if (strcaselesscmp(cp, "COMPUTECHECKSUM") == 0 || strcaselesscmp(cp, "COMPUTEGBCHECKSUM") == 0) {
    no_library_files(".COMPUTEGBCHECKSUM");
    
    if (g_gbheader_defined != 0)
      print_error(".COMPUTEGBCHECKSUM is unnecessary when GBHEADER is defined.\n", ERROR_WRN);

    g_computechecksum_defined = 1;

    return SUCCEEDED;
  }

  /* COMPUTEGBCOMPLEMENTCHECK */

  if (strcaselesscmp(cp, "COMPUTEGBCOMPLEMENTCHECK") == 0 || strcaselesscmp(cp, "COMPUTECOMPLEMENTCHECK") == 0) {
    no_library_files(".COMPUTEGBCOMPLEMENTCHECK");
    
    if (g_gbheader_defined != 0)
      print_error(".COMPUTEGBCOMPLEMENTCHECK is unnecessary when GBHEADER is defined.\n", ERROR_WRN);

    g_computecomplementcheck_defined = 1;

    return SUCCEEDED;
  }
  
#endif

#ifdef W65816

  /* COMPUTESNESCHECKSUM */

  if (strcaselesscmp(cp, "COMPUTESNESCHECKSUM") == 0) {
    no_library_files(".COMPUTESNESCHECKSUM");
    
    if (g_hirom_defined == 0 && g_lorom_defined == 0 && g_exhirom_defined == 0 && g_exlorom_defined == 0) {
      print_error(".COMPUTESNESCHECKSUM needs .LOROM, .HIROM or .EXHIROM defined earlier.\n", ERROR_DIR);
      return FAILED;
    }
    if (g_snesheader_defined != 0) 
      print_error(".COMPUTESNESCHECKSUM is unnecessary when .SNESHEADER defined.\n", ERROR_WRN);

    g_computesneschecksum_defined = 1;

    return SUCCEEDED;
  }

#endif

#ifdef Z80

  /* COMPUTESMSCHECKSUM */

  if (strcaselesscmp(cp, "COMPUTESMSCHECKSUM") == 0) {
    no_library_files(".COMPUTESMSCHECKSUM");

    g_computesmschecksum_defined++;

    return SUCCEEDED;
  }

  /* SMSTAG */

  if (strcaselesscmp(cp, "SMSTAG") == 0) {
    no_library_files(".SMSTAG");

    g_smstag_defined++;
    g_computesmschecksum_defined++;

    return SUCCEEDED;
  }

  /* SMSHEADER */
  if (strcmp(cp, "SMSHEADER") == 0)
    return directive_smsheader();
  
  /* SDSCTAG */

  if (strcaselesscmp(cp, "SDSCTAG") == 0)
    return directive_sdsctag();

#endif

  /* MACRO */

  if (strcaselesscmp(cp, "MACRO") == 0)
    return directive_macro();

  /* REPT/REPEAT */

  if (strcaselesscmp(cp, "REPT") == 0 || strcaselesscmp(cp, "REPEAT") == 0)
    return directive_rept_repeat();

  /* ENDR */

  if (strcaselesscmp(cp, "ENDR") == 0) {

    struct repeat_runtime *rr;
    
    if (g_repeat_active == 0) {
      print_error("There is no open repetition.\n", ERROR_DIR);
      return FAILED;
    }

    rr = &g_repeat_stack[g_repeat_active - 1];

    rr->counter--;
    if (rr->counter == 0) {
      g_repeat_active--;
      
      /* repeat end */
      fprintf(g_file_out_ptr, "J ");

      if (strlen(rr->index_name) > 0) {
        if (undefine(rr->index_name) == FAILED)
          return FAILED;
      }
      return SUCCEEDED;
    }

    rr->repeats++;
    if (strlen(rr->index_name) > 0) {
      if (redefine(rr->index_name, (double)rr->repeats, NULL, DEFINITION_TYPE_VALUE, 0) == FAILED)
        return FAILED;
    }
    
    g_source_pointer = rr->start;
    g_active_file_info_last->line_current = rr->start_line;

    return SUCCEEDED;
  }

  /* CHANGEFILE (INTERNAL) */
  if (strcaselesscmp(cp, "CHANGEFILE") == 0) {
    q = input_number();
    if (q != SUCCEEDED) {
      print_error("Internal error in (internal) .CHANGEFILE. Please submit a bug report...\n", ERROR_DIR);
      return FAILED;
    }
    
    g_active_file_info_tmp = calloc(sizeof(struct active_file_info), 1);
    if (g_active_file_info_tmp == NULL) {
      snprintf(g_error_message, sizeof(g_error_message), "Out of memory while trying allocate error tracking data structure.\n");
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }
    g_active_file_info_tmp->next = NULL;

    if (g_active_file_info_first == NULL) {
      g_active_file_info_first = g_active_file_info_tmp;
      g_active_file_info_last = g_active_file_info_tmp;
      g_active_file_info_tmp->prev = NULL;
    }
    else {
      g_active_file_info_tmp->prev = g_active_file_info_last;
      g_active_file_info_last->next = g_active_file_info_tmp;
      g_active_file_info_last = g_active_file_info_tmp;
    }

    g_active_file_info_tmp->line_current = 0;
    g_active_file_info_tmp->filename_id = d;

    if (g_extra_definitions == ON) {
      g_file_name_info_tmp = g_file_name_info_first;
      while (g_file_name_info_tmp != NULL) {
        if (g_file_name_info_tmp->id == d)
          break;
        g_file_name_info_tmp = g_file_name_info_tmp->next;
      }

      if (g_file_name_info_tmp == NULL) {
        snprintf(g_error_message, sizeof(g_error_message), "Internal error: Could not find the name of file %d.\n", d);
        print_error(g_error_message, ERROR_DIR);
        return FAILED;
      }

      redefine("WLA_FILENAME", 0.0, g_file_name_info_tmp->name, DEFINITION_TYPE_STRING, (int)strlen(g_file_name_info_tmp->name));
      redefine("wla_filename", 0.0, g_file_name_info_tmp->name, DEFINITION_TYPE_STRING, (int)strlen(g_file_name_info_tmp->name));
    }

    /* output the file id */
    fprintf(g_file_out_ptr, "f%d ", g_active_file_info_tmp->filename_id);
    
    g_open_files++;

    if (compare_next_token("NAMESPACE") == SUCCEEDED) {
      skip_next_token();

      g_expect_calculations = NO;
      q = input_number();
      g_expect_calculations = YES;
    
      if (q != INPUT_NUMBER_STRING && q != INPUT_NUMBER_ADDRESS_LABEL) {
        print_error("Internal error: Namespace string is missing.\n", ERROR_DIR);
        return FAILED;
      }

      strcpy(g_active_file_info_tmp->namespace, g_label);

      fprintf(g_file_out_ptr, "t1 %s ", g_active_file_info_tmp->namespace);
    }
    else if (compare_next_token("NONAMESPACE") == SUCCEEDED) {
      skip_next_token();
      
      g_active_file_info_tmp->namespace[0] = 0;

      fprintf(g_file_out_ptr, "t0 ");
    }
    else {
      print_error("Internal error: NAMESPACE/NONAMESPACE is missing.\n", ERROR_DIR);
      return FAILED;
    }
    
    return SUCCEEDED;
  }

  /* E (INTERNAL) */

  if (strcaselesscmp(cp, "E") == 0) {
    if (g_active_file_info_last != NULL) {
      g_active_file_info_tmp = g_active_file_info_last;
      g_active_file_info_last = g_active_file_info_last->prev;
      free(g_active_file_info_tmp);

      if (g_active_file_info_last == NULL)
        g_active_file_info_first = NULL;
      else {
        fprintf(g_file_out_ptr, "f%d ", g_active_file_info_last->filename_id);

        if (g_active_file_info_last->namespace[0] == 0)
          fprintf(g_file_out_ptr, "t0 ");
        else
          fprintf(g_file_out_ptr, "t1 %s ", g_active_file_info_last->namespace);      
      }
    }

    /* fix the line */
    if (g_active_file_info_last != NULL)
      g_active_file_info_last->line_current--;

    fprintf(g_file_out_ptr, "E ");
    g_open_files--;
    if (g_open_files == 0)
      return EVALUATE_TOKEN_EOP;

    if (g_extra_definitions == ON) {
      redefine("WLA_FILENAME", 0.0, get_file_name(g_active_file_info_last->filename_id), DEFINITION_TYPE_STRING,
               (int)strlen(get_file_name(g_active_file_info_last->filename_id)));
      redefine("wla_filename", 0.0, get_file_name(g_active_file_info_last->filename_id), DEFINITION_TYPE_STRING,
               (int)strlen(get_file_name(g_active_file_info_last->filename_id)));
    }

    return SUCCEEDED;
  }

  /* M */

  if (strcaselesscmp(cp, "M") == 0) {
    if (g_line_count_status == OFF)
      g_line_count_status = ON;
    else
      g_line_count_status = OFF;
    return SUCCEEDED;
  }

#ifdef GB

  /* ROMGBC */

  if (strcaselesscmp(cp, "ROMGBC") == 0) {
    no_library_files(".ROMGBC");
    
    if (g_romdmg != 0) {
      print_error(".ROMDMG was defined prior to .ROMGBC.\n", ERROR_DIR);
      return FAILED;
    }
    else if (g_romgbc == 2) {
      print_error(".ROMGBCONLY was defined prior to .ROMGBC.\n", ERROR_DIR);
      return FAILED;
    }

    g_romgbc = 1;

    return SUCCEEDED;
  }

  /* ROMGBCONLY */

  if (strcaselesscmp(cp, "ROMGBCONLY") == 0) {
    no_library_files(".ROMGBCONLY");

    if (g_romdmg != 0) {
      print_error(".ROMDMG was defined prior to .ROMGBCONLY.\n", ERROR_DIR);
      return FAILED;
    }
    else if (g_romgbc == 1) {
      print_error(".ROMGBC was defined prior to .ROMGBCONLY.\n", ERROR_DIR);
      return FAILED;
    }

    g_romgbc = 2;

    return SUCCEEDED;
  }

  /* ROMDMG */

  if (strcaselesscmp(cp, "ROMDMG") == 0) {
    no_library_files(".ROMDMG");
    
    if (g_romgbc == 1) {
      print_error(".ROMGBC was defined prior to .ROMDMG.\n", ERROR_DIR);
      return FAILED;
    }
    else if (g_romgbc == 2) {
      print_error(".ROMGBCONLY was defined prior to .ROMDMG.\n", ERROR_DIR);
      return FAILED;
    }
    else if (g_romsgb != 0) {
      print_error(".ROMDMG and .ROMSGB cannot be mixed.\n", ERROR_DIR);
      return FAILED;
    }

    g_romdmg = 1;

    return SUCCEEDED;
  }

  /* ROMSGB */

  if (strcaselesscmp(cp, "ROMSGB") == 0) {
    no_library_files(".ROMSGB");
    
    if (g_romdmg != 0) {
      print_error(".ROMDMG and .ROMSGB cannot be mixed.\n", ERROR_DIR);
      return FAILED;
    }

    g_romsgb = 1;

    return SUCCEEDED;
  }
#endif

  /* ROMBANKSIZE */

  if (strcaselesscmp(cp, "ROMBANKSIZE") == 0 || strcaselesscmp(cp, "BANKSIZE") == 0) {
    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || d < 0) {
      print_error(".ROMBANKSIZE needs a positive integer value.\n", ERROR_DIR);
      return FAILED;
    }

    if (g_banksize_defined != 0) {
      if (g_banksize != d) {
        print_error(".ROMBANKSIZE was defined for the second time.\n", ERROR_DIR);
        return FAILED;
      }
      return SUCCEEDED;
    }

    g_banksize = d;
    g_banksize_defined = 1;

    return SUCCEEDED;
  }

  /* ENDM */

  if (strcaselesscmp(cp, "ENDM") == 0)
    return directive_endm();

  /* BASE */

  if (strcaselesscmp(cp, "BASE") == 0) {
    no_library_files(".BASE definitions");

    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || d < 0) {
      print_error(".BASE number must be zero or positive.\n", ERROR_DIR);
      return FAILED;
    }

    fprintf(g_file_out_ptr, "b%d ", d);

    return SUCCEEDED;
  }

#if defined(MCS6502) || defined(MCS6510) || defined(W65816) || defined(WDC65C02) || defined(CSG65CE02) || defined(HUC6280) || defined(MC6800) || defined(MC6801) || defined(MC6809)

  /* 8BIT */

  if (strcaselesscmp(cp, "8BIT") == 0) {
    g_xbit_size = 8;
    return SUCCEEDED;
  }

  /* 16BIT */

  if (strcaselesscmp(cp, "16BIT") == 0) {
    g_xbit_size = 16;
    return SUCCEEDED;
  }

#endif

#ifdef W65816

  /* 24BIT */

  if (strcaselesscmp(cp, "24BIT") == 0) {
    g_xbit_size = 24;
    return SUCCEEDED;
  }

  /* INDEX */

  if (strcaselesscmp(cp, "INDEX") == 0) {
    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || !(d == 8 || d == 16)) {
      print_error("The index size must be 8 or 16.\n", ERROR_DIR);
      return FAILED;
    }

    g_index_size = d;

    return SUCCEEDED;
  }

  /* ACCU */

  if (strcaselesscmp(cp, "ACCU") == 0) {
    q = input_number();

    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED || !(d == 8 || d == 16)) {
      print_error("The accumulator size must be 8 or 16.\n", ERROR_DIR);
      return FAILED;
    }

    g_accu_size = d;

    return SUCCEEDED;
  }

  /* SMC */

  if (strcaselesscmp(cp, "SMC") == 0) {
    no_library_files(".SMC");

    g_smc_defined++;
    g_snes_mode++;

    return SUCCEEDED;
  }

  /* HIROM */

  if (strcaselesscmp(cp, "HIROM") == 0) {
    no_library_files(".HIROM");

    if (g_lorom_defined != 0 || g_exlorom_defined != 0 || g_exhirom_defined != 0) {
      give_snes_rom_mode_defined_error(".HIROM");
      return FAILED;
    }

    g_hirom_defined++;
    g_snes_mode++;

    return SUCCEEDED;
  }

  /* EXHIROM */

  if (strcaselesscmp(cp, "EXHIROM") == 0) {
    no_library_files(".EXHIROM");

    if (g_lorom_defined != 0 || g_exlorom_defined != 0 || g_hirom_defined != 0) {
      give_snes_rom_mode_defined_error(".EXHIROM");
      return FAILED;
    }

    g_exhirom_defined++;
    g_snes_mode++;

    return SUCCEEDED;
  }
  
  /* LOROM */

  if (strcaselesscmp(cp, "LOROM") == 0) {
    no_library_files(".LOROM");

    if (g_hirom_defined != 0 || g_exlorom_defined != 0 || g_exhirom_defined != 0) {
      give_snes_rom_mode_defined_error(".LOROM");
      return FAILED;
    }

    g_lorom_defined++;
    g_snes_mode++;

    return SUCCEEDED;
  }

  /* EXLOROM */
  /*
    if (strcaselesscmp(cp, "EXLOROM") == 0) {
    no_library_files(".EXLOROM");

    if (g_hirom_defined != 0 || g_lorom_defined != 0 || g_exhirom_defined != 0) {
    give_snes_rom_mode_defined_error(".EXLOROM");
    return FAILED;
    }

    g_exlorom_defined++;
    snes_mode++;

    return SUCCEEDED;
    }
  */
  /* SLOWROM */

  if (strcaselesscmp(cp, "SLOWROM") == 0) {
    no_library_files(".SLOWROM");
    
    if (g_fastrom_defined != 0) {
      print_error(".FASTROM was defined prior to .SLOWROM.\n", ERROR_DIR);
      return FAILED;
    }

    g_slowrom_defined++;
    g_snes_mode++;

    return SUCCEEDED;
  }

  /* FASTROM */

  if (strcaselesscmp(cp, "FASTROM") == 0) {
    no_library_files(".FASTROM");
    
    if (g_slowrom_defined != 0) {
      print_error(".SLOWROM was defined prior to .FASTROM.\n", ERROR_DIR);
      return FAILED;
    }

    g_fastrom_defined++;
    g_snes_mode++;

    return SUCCEEDED;
  }

  /* SNESHEADER */

  if (strcaselesscmp(cp, "SNESHEADER") == 0)
    return directive_snesheader();

  /* SNESNATIVEVECTOR */

  if (strcaselesscmp(cp, "SNESNATIVEVECTOR") == 0)
    return directive_snesnativevector();

  /* SNESEMUVECTOR */

  if (strcaselesscmp(cp, "SNESEMUVECTOR") == 0)
    return directive_snesemuvector();

#endif

  /* PRINT */

  if (strcaselesscmp(cp, "PRINT") == 0)
    return directive_print();
  
  /* PRINTT */

  if (strcaselesscmp(cp, "PRINTT") == 0)
    return directive_printt();

  /* PRINTV */

  if (strcaselesscmp(cp, "PRINTV") == 0)
    return directive_printv();

  /* SEED */

  if (strcaselesscmp(cp, "SEED") == 0) {
    q = input_number();
    if (q == FAILED)
      return FAILED;
    if (q != SUCCEEDED) {
      print_error(".SEED needs a seed value for the randon number generator.\n", ERROR_DIR);
      return FAILED;
    }

    /* reseed the random number generator */
    init_genrand(d);

    return SUCCEEDED;
  }

  /* DBRND/DWRND */

  if (strcaselesscmp(cp, "DBRND") == 0 || strcaselesscmp(cp, "DWRND") == 0)
    return directive_dbrnd_dwrnd();

  /* DWSIN/DBSIN/DWCOS/DBCOS */

  if (strcaselesscmp(cp, "DWSIN") == 0 || strcaselesscmp(cp, "DBSIN") == 0 || strcaselesscmp(cp, "DWCOS") == 0 || strcaselesscmp(cp, "DBCOS") == 0)
    return directive_dwsin_dbsin_dwcos_dbcos();

  /* FAIL */

  if (strcaselesscmp(cp, "FAIL") == 0) {
    print_error("HALT: .FAIL found.\n", ERROR_NONE);

    /* make a silent exit */
    exit(0);
  }

  /* UNDEF/UNDEFINE */

  if (strcaselesscmp(cp, "UNDEF") == 0 || strcaselesscmp(cp, "UNDEFINE") == 0)
    return directive_undef_undefine();

  /* ASM */

  if (strcaselesscmp(cp, "ASM") == 0)
    return SUCCEEDED;

  /* ENDASM */

  if (strcaselesscmp(cp, "ENDASM") == 0) {

    int endasm = 1, x;

    while (1) {
      x = g_source_pointer;
      q = get_next_token();
      if (q == GET_NEXT_TOKEN_STRING)
        continue;

      /* end of file? */
      if (strcmp(tmp, ".E") == 0) {
        g_source_pointer = x;
        return SUCCEEDED;
      }
      if (strcaselesscmp(tmp, ".ASM") == 0) {
        endasm--;
        if (endasm == 0)
          return SUCCEEDED;
      }
      if (strcaselesscmp(tmp, ".ENDASM") == 0)
        endasm++;
    }
  }

  /* STRINGMAPTABLE */

  if (strcaselesscmp(cp, "STRINGMAPTABLE") == 0)
    return directive_stringmap_table();

  /* STRINGMAP */

  if (strcaselesscmp(cp, "STRINGMAP") == 0)
    return directive_stringmap();

  return DIRECTIVE_NOT_IDENTIFIED;
}


/* parses only "if" directives. */
/* this is separate from parse_directive so that enums and ramsections can reuse this */
int parse_if_directive(void) {

  char bak[256];
  int q;

  /* ELSE */

  if (strcaselesscmp(cp, "ELSE") == 0) {

    int m, r;

    if (g_ifdef == 0) {
      print_error("There must be .IFxxx before .ELSE.\n", ERROR_DIR);
      return FAILED;
    }

    /* find the next compiling point */
    r = 1;
    m = g_macro_active;
    /* disable macro decoding */
    g_macro_active = 0;
    while (get_next_token() != FAILED) {
      if (tmp[0] == '.') {
        if (strcaselesscmp(cp, "ENDIF") == 0)
          r--;
        if (strcaselesscmp(cp, "E") == 0)
          break;
        if (strcaselesscmp(cp, "IFDEF") == 0 || strcaselesscmp(cp, "IFNDEF") == 0 || strcaselesscmp(cp, "IFGR") == 0 || strcaselesscmp(cp, "IFLE") == 0 || strcaselesscmp(cp, "IFEQ") == 0 ||
            strcaselesscmp(cp, "IFNEQ") == 0 || strcaselesscmp(cp, "IFDEFM") == 0 || strcaselesscmp(cp, "IFNDEFM") == 0 || strcaselesscmp(cp, "IF") == 0 || strcaselesscmp(cp, "IFEXISTS") == 0 ||
            strcaselesscmp(cp, "IFGREQ") == 0 || strcaselesscmp(cp, "IFLEEQ") == 0)
          r++;
      }
      if (r == 0) {
        g_ifdef--;
        g_macro_active = m;
        return SUCCEEDED;
      }
    }

    print_error(".ELSE must end to .ENDIF.\n", ERROR_DIR);
    return FAILED;
  }

  /* ENDIF */

  if (strcaselesscmp(cp, "ENDIF") == 0) {
    if (g_ifdef == 0) {
      print_error(".ENDIF was given before any .IF directive.\n", ERROR_DIR);
      return FAILED;
    }

    g_ifdef--;
    return SUCCEEDED;
  }

  /* IFDEF */

  if (strcaselesscmp(cp, "IFDEF") == 0) {

    struct definition *d;

    if (get_next_token() == FAILED)
      return FAILED;

    hashmap_get(g_defines_map, tmp, (void*)&d);
    if (d != NULL) {
      g_ifdef++;
      return SUCCEEDED;
    }

    return find_next_point("IFDEF");
  }

  /* IF */

  if (strcaselesscmp(cp, "IF") == 0) {

    char k[256];
    int y, o, s;

    q = input_number();
    if (q != SUCCEEDED && q != INPUT_NUMBER_STRING && q != INPUT_NUMBER_ADDRESS_LABEL) {
      snprintf(g_error_message, sizeof(g_error_message), ".IF needs immediate data, string or an address label.\n");
      print_error(g_error_message, ERROR_INP);
      return FAILED;
    }

    strncpy(k, g_label, 255);
    k[255] = 0;
    y = d;
    s = q;

    if (get_next_token() == FAILED)
      return FAILED;

    if (strcmp(tmp, "<") == 0)
      o = 0;
    else if (strcmp(tmp, ">") == 0)
      o = 1;
    else if (strcmp(tmp, "==") == 0)
      o = 2;
    else if (strcmp(tmp, "!=") == 0)
      o = 3;
    else if (strcmp(tmp, ">=") == 0)
      o = 4;
    else if (strcmp(tmp, "<=") == 0)
      o = 5;
    else {
      print_error(".IF needs an operator. Supported operators are '<', '>', '>=', '<=', '!=' and '=='.\n", ERROR_INP);
      return FAILED;
    }

    q = input_number();
    if (q != SUCCEEDED && q != INPUT_NUMBER_STRING && q != INPUT_NUMBER_ADDRESS_LABEL) {
      snprintf(g_error_message, sizeof(g_error_message), ".IF needs immediate data, string or an address label.\n");
      print_error(g_error_message, ERROR_INP);
      return FAILED;
    }

    /* different types? */
    if (s != q) {
      print_error("The types of the compared things must be the same.\n", ERROR_INP);
      return FAILED;
    }

    /* values? */
    if (s == SUCCEEDED) {
      if ((o == 0 && y < d) || (o == 1 && y > d) || (o == 2 && y == d) || (o == 3 && y != d) || (o == 4 && y >= d) || (o == 5 && y <= d))
        q = SUCCEEDED;
      else
        q = FAILED;
    }
    /* strings? */
    else {
      if ((o == 0 && strcmp(k, g_label) < 0) || (o == 1 && strcmp(k, g_label) > 0) || (o == 2 && strcmp(k, g_label) == 0) || (o == 3 && strcmp(k, g_label) != 0) || (o == 4 && strcmp(k, g_label) >= 0) || (o == 5 && strcmp(k, g_label) <= 0))
        q = SUCCEEDED;
      else
        q = FAILED;
    }

    if (q == SUCCEEDED) {
      g_ifdef++;
      return SUCCEEDED;
    }
    else
      return find_next_point("IF");
  }

  /* IFGR/IFLE/IFEQ/IFNEQ/IFGREQ/IFLEEQ */

  if (strcaselesscmp(cp, "IFGR") == 0 || strcaselesscmp(cp, "IFLE") == 0 || strcaselesscmp(cp, "IFEQ") == 0 || strcaselesscmp(cp, "IFNEQ") == 0 || strcaselesscmp(cp, "IFGREQ") == 0 || strcaselesscmp(cp, "IFLEEQ") == 0) {

    char k[256];
    int y, o, s;

    strcpy(bak, cp);

    if (strcmp(&cp[2], "LE") == 0)
      o = 0;
    else if (strcmp(&cp[2], "GR") == 0)
      o = 1;
    else if (strcmp(&cp[2], "EQ") == 0)
      o = 2;
    else if (strcmp(&cp[2], "NEQ") == 0)
      o = 3;
    else if (strcmp(&cp[2], "GREQ") == 0)
      o = 4;
    else
      o = 5;

    q = input_number();
    if (q != SUCCEEDED && q != INPUT_NUMBER_STRING && q != INPUT_NUMBER_ADDRESS_LABEL) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s needs immediate data.\n", bak);
      print_error(g_error_message, ERROR_INP);
      return FAILED;
    }

    strncpy(k, g_label, 255);
    k[255] = 0;
    y = d;
    s = q;

    q = input_number();
    if (q != SUCCEEDED && q != INPUT_NUMBER_STRING && q != INPUT_NUMBER_ADDRESS_LABEL) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s needs immediate data.\n", bak);
      print_error(g_error_message, ERROR_INP);
      return FAILED;
    }

    /* different types? */
    if (s != q) {
      print_error("The types of the compared things must be the same.\n", ERROR_INP);
      return FAILED;
    }

    /* values? */
    if (s == SUCCEEDED) {
      if ((o == 0 && y < d) || (o == 1 && y > d) || (o == 2 && y == d) || (o == 3 && y != d) || (o == 4 && y >= d) || (o == 5 && y <= d))
        q = SUCCEEDED;
      else
        q = FAILED;
    }
    /* strings? */
    else {
      if ((o == 0 && strcmp(k, g_label) < 0) || (o == 1 && strcmp(k, g_label) > 0) || (o == 2 && strcmp(k, g_label) == 0) || (o == 3 && strcmp(k, g_label) != 0) || (o == 4 && strcmp(k, g_label) >= 0) || (o == 5 && strcmp(k, g_label) <= 0))
        q = SUCCEEDED;
      else
        q = FAILED;
    }

    if (q == SUCCEEDED) {
      g_ifdef++;
      return SUCCEEDED;
    }
    else {
      strcpy(k, cp);
      return find_next_point(k);
    }
  }

  /* IFEXISTS */

  if (strcaselesscmp(cp, "IFEXISTS") == 0) {

    FILE *f;

    g_expect_calculations = NO;
    inz = input_number();
    g_expect_calculations = YES;

    if (inz != INPUT_NUMBER_STRING && inz != INPUT_NUMBER_ADDRESS_LABEL) {
      print_error(".IFEXISTS needs a file name string.\n", ERROR_DIR);
      return FAILED;
    }

    f = fopen(g_label, "r");
    if (f == NULL)
      return find_next_point("IFEXISTS");

    fclose(f);
    g_ifdef++;

    return SUCCEEDED;
  }

  /* IFNDEF */

  if (strcaselesscmp(cp, "IFNDEF") == 0) {

    struct definition *d;

    if (get_next_token() == FAILED)
      return FAILED;

    hashmap_get(g_defines_map, tmp, (void*)&d);
    if (d != NULL) {
      strcpy(g_error_message, cp);
      return find_next_point(g_error_message);
    }

    g_ifdef++;

    return SUCCEEDED;
  }

  /* IFDEFM/IFNDEFM */

  if (strcaselesscmp(cp, "IFDEFM") == 0 || strcaselesscmp(cp, "IFNDEFM") == 0) {

    int k, o;
    char e;

    strcpy(bak, cp);

    if (g_macro_active == 0) {
      snprintf(g_error_message, sizeof(g_error_message), ".%s can be only used inside a macro.\n", bak);
      print_error(g_error_message, ERROR_DIR);
      return FAILED;
    }

    if (cp[2] == 'N')
      o = 0;
    else
      o = 1;

    for (; g_source_pointer < g_size; g_source_pointer++) {

      if (g_buffer[g_source_pointer] == 0x0A)
        break;
      else if (g_buffer[g_source_pointer] == '\\') {
        e = g_buffer[++g_source_pointer];
        if (e >= '0' && e <= '9') {
          d = (e - '0') * 10;
          for (k = 2; k < 8; k++, d *= 10) {
            e = g_buffer[++g_source_pointer];
            if (e >= '0' && e <= '9')
              d += e - '0';
            else
              break;
          }

          d /= 10;
          if ((o == 0 && g_macro_runtime_current->supplied_arguments < d) ||
              (o == 1 && g_macro_runtime_current->supplied_arguments >= d)) {
            g_ifdef++;
            return SUCCEEDED;
          }
          else {
            strcpy(g_error_message, cp);
            return find_next_point(g_error_message);
          }
        }
        break;
      }
    }

    snprintf(g_error_message, sizeof(g_error_message), ".%s needs an argument.\n", bak);
    print_error(g_error_message, ERROR_DIR);

    return FAILED;
  }

  /* neither success nor failure (didn't match any "if" directives) */
  return -1;
}


int find_next_point(char *name) {

  int depth, m, line_current;

  line_current = g_active_file_info_last->line_current;
  /* find the next compiling point */
  depth = 1;
  m = g_macro_active;
  /* disable macro decoding */
  g_macro_active = 0;
  while (get_next_token() != FAILED) {
    if (tmp[0] == '.') {
      if (strcaselesscmp(cp, "ENDIF") == 0)
        depth--;
      if (strcaselesscmp(cp, "ELSE") == 0 && depth == 1)
        depth--;
      if (strcaselesscmp(cp, "E") == 0)
        break;
      if (strcaselesscmp(cp, "IFDEF") == 0 || strcaselesscmp(cp, "IFNDEF") == 0 || strcaselesscmp(cp, "IFGR") == 0 || strcaselesscmp(cp, "IFLE") == 0 ||
          strcaselesscmp(cp, "IFEQ") == 0 || strcaselesscmp(cp, "IFNEQ") == 0 || strcaselesscmp(cp, "IFDEFM") == 0 || strcaselesscmp(cp, "IFNDEFM") == 0 ||
          strcaselesscmp(cp, "IF") == 0 || strcaselesscmp(cp, "IFGREQ") == 0 || strcaselesscmp(cp, "IFLEEQ") == 0 || strcaselesscmp(cp, "IFEXISTS") == 0)
        depth++;
    }
    if (depth == 0) {
      if (strcaselesscmp(cp, "ELSE") == 0)
        g_ifdef++;
      g_macro_active = m;
      return SUCCEEDED;
    }
  }

  /* return the condition's line number */
  g_active_file_info_last->line_current = line_current;
  snprintf(g_error_message, sizeof(g_error_message), ".%s must end to .ENDIF/.ELSE.\n", name);
  print_error(g_error_message, ERROR_DIR);

  return FAILED;
}


#ifndef GB

void delete_stack(struct stack *s) {

  if (s == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Deleting a non-existing computation stack. This can lead to problems.\n");
    print_error(g_error_message, ERROR_WRN);
    return;
  }

  free(s->stack);
  free(s);
}

#endif


void parse_print_string(char *input, char *output, int output_size) {

  int s, u;

  for (s = 0, u = 0; input[s] != 0 && u < output_size-1; ) {
    if (input[s] == '\\' && input[s + 1] == 'n') {
#ifdef MSDOS
      output[u++] = '\r';
      output[u++] = '\n';
#else
      output[u++] = '\n';
#endif
      s += 2;
    }
    else if (input[s] == '\\' && input[s + 1] == '\\') {
      output[u++] = '\\';
      s += 2;
    }
    else
      output[u++] = input[s++];
  }

  output[u] = 0;
}


int is_reserved_definition(char *t) {

  if (strcaselesscmp(t, "WLA_TIME") == 0 ||
      strcaselesscmp(t, "WLA_VERSION") == 0 ||
      strcaselesscmp(t, "WLA_FILENAME") == 0 ||
      strcaselesscmp(t, "NARGS") == 0 ||
      strcaselesscmp(t, "CADDR") == 0)
    return YES;

  return NO;
}


int get_new_definition_data(int *b, char *c, int *size, double *data, int *export) {

  int a, x, n, s;

  *export = NO;
  
  x = input_number();
  a = x;
  n = 0;
  s = 0;

  if (x == INPUT_NUMBER_ADDRESS_LABEL) {
    /* address label -> stack */
    stack_create_label_stack(g_label);
    x = INPUT_NUMBER_STACK;
  }

  if (x == INPUT_NUMBER_STACK) {
    *b = g_stacks_tmp->id;
    g_stacks_tmp->position = STACK_POSITION_DEFINITION;

    /* export the definition? */
    if (compare_next_token("EXPORT") == SUCCEEDED) {
      skip_next_token();
      *export = YES;
    }

    x = input_number();
    if (x != INPUT_NUMBER_EOL) {
      print_error("A computation cannot be output as a string.\n", ERROR_DIR);
      return SUCCEEDED;
    }
    next_line();
    return INPUT_NUMBER_STACK;
  }

  while (x != INPUT_NUMBER_EOL) {
    /* the first fetch will conserve both classes */
    if (n == 0) {
      if (x == SUCCEEDED)
        *b = d;
      else if (x == INPUT_NUMBER_STRING) {
        strcpy(c, g_label);
        s = (int)strlen(g_label);
      }
      else if (x == INPUT_NUMBER_FLOAT) {
        *data = g_parsed_double;
        *b = (int)g_parsed_double;
      }
      else
        return x;

      n++;

      /* export the definition? */
      if (compare_next_token("EXPORT") == SUCCEEDED) {
        skip_next_token();
        *export = YES;
      }

      x = input_number();
      continue;
    }
    /* the next fetches will produce strings */
    else if (n == 1 && (a == SUCCEEDED || a == INPUT_NUMBER_FLOAT)) {
      if (*b > 255) {
        c[0] = (*b & 255);
        c[1] = (*b >> 8) & 255;
        c[2] = 0;
        s = 2;
      }
      else {
        c[0] = *b;
        c[1] = 0;
        s = 1;
      }
      a = INPUT_NUMBER_STRING;
    }

    /* transform floats to integers */
    if (x == INPUT_NUMBER_FLOAT) {
      d = (int)g_parsed_double;
      x = SUCCEEDED;
    }

    if (x == INPUT_NUMBER_STRING) {
      strcpy(&c[s], g_label);
      s += (int)strlen(g_label);
    }
    else if (x == SUCCEEDED) {
      if (d > 255) {
        c[s] = (d & 255);
        c[s + 1] = (d >> 8) & 255;
        c[s + 2] = 0;
        s += 2;
      }
      else {
        c[s] = d;
        c[s + 1] = 0;
        s++;
      }
    }
    else
      return x;

    n++;

    /* export the definition? */
    if (compare_next_token("EXPORT") == SUCCEEDED) {
      skip_next_token();
      *export = YES;
    }

    x = input_number();
  }

  next_line();

  if (a == INPUT_NUMBER_STRING)
    c[s] = 0;

  *size = s;

  return a;
}


int export_a_definition(char *name) {

  struct export_def *export;

  /* don't export it twice or more often */
  export = g_export_first;
  while (export != NULL) {
    if (strcmp(export->name, name) == 0) {
      snprintf(g_error_message, sizeof(g_error_message), "\"%s\" was .EXPORTed for the second time.\n", name);
      print_error(g_error_message, ERROR_WRN);
      return SUCCEEDED;
    }
    export = export->next;
  }

  export = calloc(sizeof(struct export_def), 1);
  if (export == NULL) {
    snprintf(g_error_message, sizeof(g_error_message), "Out of memory while allocating room for \".EXPORT %s\".\n", name);
    print_error(g_error_message, ERROR_DIR);
    return FAILED;
  }

  strcpy(export->name, name);
  export->next = NULL;

  if (g_export_first == NULL) {
    g_export_first = export;
    g_export_last = export;
  }
  else {
    g_export_last->next = export;
    g_export_last = export;
  }

  return SUCCEEDED;
}


/* store the labels in a label stack in which g_label_stack[0] is the base level label,
   g_label_stack[1] is the first child, g_label_stack[2] is the second child, and so on... */
int add_label_to_label_stack(char *l) {

  int level = 0, q;

  /* skip anonymous labels */
  if (is_label_anonymous(l) == SUCCEEDED)
    return SUCCEEDED;

  for (q = 0; q < MAX_NAME_LENGTH; q++) {
    if (l[q] == '@')
      level++;
    else
      break;
  }

  if (level >= 256) {
    print_error("ADD_LABEL_TO_LABEL_STACK: Out of label stack depth. Can handle only 255 child labels...\n", ERROR_ERR);
    return FAILED;
  }

  if (level == 0) {
    /* resetting level 0 label clears all the other levels */
    for (q = 0; q < 256; q++)
      g_label_stack[q][0] = 0;
    strcpy(g_label_stack[0], l);
  }
  else
    strcpy(g_label_stack[level], &l[level-1]);

  /*
    fprintf(stderr, "*************************************\n");
    fprintf(stderr, "LABEL STACK:\n");
    for (q = 0; q < 256; q++) {
    if (g_label_stack[q][0] != 0)
    fprintf(stderr, "%s LEVEL %d\n", g_label_stack[q], q);
    }
    fprintf(stderr, "*************************************\n");
  */

  return SUCCEEDED;
}


/* get the full version of a (possibly child) label */
int get_full_label(char *l, char *out) {

  char *error_message = "GET_FULL_LABEL: Constructed label size will be >= MAX_NAME_LENGTH. Edit the define in shared.h, recompile WLA and try again...\n";
  int level = 0, q;

  for (q = 0; q < MAX_NAME_LENGTH; q++) {
    if (l[q] == '@')
      level++;
    else
      break;
  }

  if (level <= 0)
    strcpy(out, l);
  else {
    /* create the full label, e.g.: "BASE@CHILD1@CHILD2" */
    strcpy(out, g_label_stack[0]);
    for (q = 1; q < level; q++) {
      if (strlen(out) + strlen(g_label_stack[q]) >= MAX_NAME_LENGTH) {
        print_error(error_message, ERROR_ERR);
        return FAILED;  
      }
      strncat(out, g_label_stack[q], MAX_NAME_LENGTH);
    }

    if (strlen(out) + strlen(&l[level-1]) >= MAX_NAME_LENGTH) {
      print_error(error_message, ERROR_ERR);
      return FAILED;    
    }

    strncat(out, &l[level-1], MAX_NAME_LENGTH);
  }

  return SUCCEEDED;
}


int add_namespace_to_string(char *s, int sizeof_s, char *type) {

  char buf[MAX_NAME_LENGTH + 1];
    
  snprintf(buf, sizeof(buf), "%s.%s", g_active_file_info_last->namespace, s);
  buf[sizeof(buf)-1] = 0;
  if (strlen(buf) >= (size_t)sizeof_s) {
    snprintf(g_error_message, sizeof(g_error_message), "The current file namespace \"%s\" cannot be added to %s's \"%s\" name - increase MAX_NAME_LENGTH in shared.h and recompile WLA.\n", g_active_file_info_last->namespace, type, tmp);
    print_error(g_error_message, ERROR_ERR);
    return FAILED;
  }

  strcpy(s, buf);

  return SUCCEEDED;
}


void generate_label(char *header, char *footer) {

  char footer2[MAX_NAME_LENGTH + 1];
  int q, o;
  
  /* check if the footer contains spaces */
  o = (int)strlen(footer);
  for (q = 0; q < o; q++) {
    if (footer[q] == ' ')
      footer2[q] = '_';
    else
      footer2[q] = footer[q];
  }
  footer2[q] = 0;

  fprintf(g_file_out_ptr, "L%s%s ", header, footer2);
}
