#include "headers/define.h"

void file_setup(_app *p_app) {
	DIR *p_dir = opendir("src/file");
	if (!p_dir) {
		fprintf(stderr, "WARNING: could not open src/file/\n");
		exit(1);
	}

	struct dirent *entry;
	int count = 0;

	while ((entry = readdir(p_dir)) != NULL) {
		char *ext = strrchr(entry->d_name, '.');
		if (!ext || strcmp(ext, ".webp") != 0) continue;

		count++;
		if (count > 1) {
			fprintf(stderr, "WARNING: more than one .webp file in src/file/\n");
			closedir(p_dir);
			exit(1);
		}

		char tmp[512];
		snprintf(tmp, sizeof(tmp), "src/file/%s", entry->d_name);
		p_app->webp.file = strdup(tmp);
	}

	closedir(p_dir);

	if (count == 0) {
		fprintf(stderr, "WARNING: no .webp file found in src/file/\n");
		exit(1);
	}

	printf("found: %s\n", p_app->webp.file);
}

void file_read(_app *p_app) {
	if (p_app->webp.file == NULL) {
		fprintf(stderr, "WARNING: no file loaded\n");
		exit(1);
	}

	FILE *p_f = fopen(p_app->webp.file, "rb");

	fseek(p_f, 0, SEEK_END);
	p_app->webp.data_size = ftell(p_f);
	rewind(p_f);

	p_app->webp.data = malloc(p_app->webp.data_size);
	fread(p_app->webp.data, 1, p_app->webp.data_size, p_f);
	fclose(p_f);
}

int main() {
	_app app = {0};
	file_setup(&app);
	file_read(&app);
}
