#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *notice = "/* This is an automatically generated file.\n"
                     " * Edits of this file will be lost at build time.\n"
                     " */\n";

struct plan {
	FILE *in;
	FILE *header;
	FILE *source;
	const char *header_path;
	const char *source_path;
	const char *name;
};

static void write_macro(FILE *header, const char *name) {

	char c;
	unsigned int i;

	for (i = 0; name[i]; i++) {
		c = name[i];
		if (isalnum(c)) {
			fprintf(header, "%c", toupper(c));
		} else {
			fprintf(header, "_");
		}
	}
}

static void write_header(struct plan *plan) {

	FILE *header;

	header = plan->header;

	fprintf(header, "%s", notice);

	fprintf(header, "\n");
	fprintf(header, "#ifndef SWANSON_RC_");
	write_macro(header, plan->header_path);
	fprintf(header, "\n");

	fprintf(header, "#define SWANSON_RC_");
	write_macro(header, plan->header_path);
	fprintf(header, "\n");

	fprintf(header, "\n");
	fprintf(header, "#ifdef __cplusplus\n");
	fprintf(header, "extern \"C\" {\n");
	fprintf(header, "#endif\n");

	fprintf(header, "\n");
	fprintf(header, "extern const void *%s;\n", plan->name);

	fprintf(header, "\n");
	fprintf(header, "extern const unsigned long int %s_size;\n", plan->name);

	fprintf(header, "\n");
	fprintf(header, "#ifdef __cplusplus\n");
	fprintf(header, "} /* extern \"C\" */\n");
	fprintf(header, "#endif\n");

	fprintf(header, "\n");
	fprintf(header, "#endif /* SWANSON_RC_");
	write_macro(header, plan->header_path);
	fprintf(header, " */\n");
}

static void write_source(struct plan *plan) {

	unsigned int i;
	unsigned int read_count;
	unsigned char buf[16];
	FILE *source;

	source = plan->source;

	fprintf(source, "%s", notice);

	fprintf(source, "\n");
	fprintf(source, "#include \"%s\"\n", plan->header_path);

	fprintf(source, "\n");
	fprintf(source, "const unsigned char %s_bytes[] = {\n", plan->name);
	while (!feof(plan->in)) {
		fprintf(source, "\t");
		read_count = fread(buf, 1, sizeof(buf), plan->in);
		for (i = 0; i < read_count; i++) {
			if ((i + 1) < read_count) {
				fprintf(source, "0x%02x, ", buf[i]);
			} else {
				fprintf(source, "0x%02x,\n", buf[i]);
			}
		}
	}
	fprintf(source, "};\n");

	fprintf(source, "\n");
	fprintf(source, "const void *%s = %s_bytes;\n", plan->name, plan->name);

	fprintf(source, "\n");
	fprintf(source, "const unsigned long int %s_size = sizeof(%s_bytes);\n", plan->name, plan->name);
}

static void execute_plan(struct plan *plan) {
	write_header(plan);
	write_source(plan);
}

int main(int argc, const char **argv) {

	int i;
	FILE *source;
	FILE *header;
	FILE *input;
	const char *field_name;
	const char *source_path;
	const char *header_path;
	const char *input_path;
	struct plan plan;

	input = NULL;
	source = NULL;
	header = NULL;
	input_path = NULL;
	source_path = NULL;
	header_path = NULL;
	field_name = NULL;

	for (i = 1; i < argc; i += 2) {
		if (strcmp(argv[i], "--header") == 0) {
			header_path = argv[i + 1];
		} else if (strcmp(argv[i], "--source") == 0) {
			source_path = argv[i + 1];
		} else if (strcmp(argv[i], "--input") == 0) {
			input_path = argv[i + 1];
		} else if (strcmp(argv[i], "--name") == 0) {
			field_name = argv[i + 1];
		} else if (strcmp(argv[i], "--help") == 0) {
			printf("Usage: %s [Options]\n", argv[0]);
			printf("\n");
			printf("Options:\n");
			printf("\t--header PATH : The header that contains the forward declarations of the variable.\n");
			printf("\t--source PATH : The source file to put the data in.\n");
			printf("\t--input PATH  : The file containing the data to put in source code.\n");
			printf("\t--name PATH   : The name of the variable containing the data.\n");
			printf("\t--help        : Print this help message.\n");
			return EXIT_FAILURE;
		} else {
			fprintf(stderr, "Unknown option '%s'.\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	if (header_path == NULL) {
		fprintf(stderr, "Must specify a header path with '--header PATH'.\n");
		return EXIT_FAILURE;
	}

	if (source_path == NULL) {
		fprintf(stderr, "Must specify source file with '--source PATH'.\n");
		return EXIT_FAILURE;
	}

	if (input_path == NULL) {
		fprintf(stderr, "Must specify input file with '--input PATH'.\n");
		return EXIT_FAILURE;
	}

	if (field_name == NULL) {
		fprintf(stderr, "Must specify variable name with '--name NAME'.\n");
		return EXIT_FAILURE;
	}

	source = fopen(source_path, "wb");
	if (source == NULL) {
		fprintf(stderr, "Failed to open source file '%s'.\n", source_path);
		return EXIT_FAILURE;
	}

	header = fopen(header_path, "wb");
	if (header == NULL) {
		fprintf(stderr, "Failed to open header file '%s'.\n", header_path);
		fclose(source);
		return EXIT_FAILURE;
	}

	input = fopen(input_path, "rb");
	if (input == NULL) {
		fprintf(stderr, "Failed to open input file '%s'.\n", input_path);
		fclose(header);
		fclose(source);
		return EXIT_FAILURE;
	}

	plan.in = input;
	plan.source = source;
	plan.source_path = source_path;
	plan.header = header;
	plan.header_path = header_path;
	plan.name = field_name;

	execute_plan(&plan);

	if (input != NULL)
		fclose(input);

	if (header != NULL)
		fclose(header);

	if (source != NULL)
		fclose(source);

	return EXIT_SUCCESS;
}
