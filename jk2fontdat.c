#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#define GLYPH_COUNT 256

typedef struct
{
	short		width;					// number of pixels wide - pitch
	short		height;					// number of scan lines - height
	short		horizAdvance;			// number of pixels to advance to the next char - xSkip
	short		horizOffset;			// x offset into space to render glyph - ignored?
	int			baseline;				// y offset - ignored?
	float		s;						// x start tex coord
	float		t;						// y start tex coord
	float		s2;						// x end tex coord
	float		t2;						// y end tex coord
} glyphInfojk2_t;

typedef struct dfontdat_s
{
	glyphInfojk2_t		mGlyphs[GLYPH_COUNT];

	short			mPointSize;
	short			mHeight;				// max height of font
	short			mAscender;
	short			mDescender;

	short			mKoreanHack;
} dfontdat_t;

#define check_failure(ret, err, msg)		\
	do {					\
		if ((ret) == (err)) {		\
			perror(msg);		\
			exit(EXIT_FAILURE);	\
		}				\
	} while(0)

// Globals

dfontdat_t jk2font;
const char *progName;

// Procedures

void PrintHelpExit(int exitCode)
{
	printf("  Usage: %s [-uzei] FILE.fontdat\n", progName);
	printf("Options:\n"
	       "  -u replace lowercase glyph data with uppercase\n"
	       "  -z zero all fields in empty glyphs\n"
	       "  -c adjust spacing in Ergoe Condensed Bold font\n"
	       "  -e export font to csv\n"
	       "  -i import font from csv\n"
	       "  -p print on terminal\n"
		);
	exit(exitCode);
}

FILE *OpenFileLoad(const char *file)
{
	if (!strcmp(file, "-")) {
		return stdin;
	} else {
		FILE *fd = fopen(file, "rb");
		check_failure(fd, NULL, "Error opening file: ");
		printf("Loading File: %s\n", file);
		return fd;
	}
}

FILE *OpenFileSave(const char *file)
{
	if (!strcmp(file, "-")) {
		return stdout;
	} else {
		FILE *fd = fopen(file, "wb");
		check_failure(fd, NULL, "Error opening file: ");
		printf("Saving File: %s\n", file);
		return fd;
	}
}

void LoadFontData(const char *file)
{
	size_t size;
	FILE *fd = OpenFileLoad(file);

	// Little endian only
	size = fread(&jk2font, 1, sizeof(jk2font), fd);

	if (size < sizeof(jk2font)) {
		printf("Font data too short.\n");
		exit(EXIT_FAILURE);
	} else if (size > sizeof(jk2font)) {
		printf("Font data too long.\n");
		exit(EXIT_FAILURE);
	}

	fclose(fd);
}

void SaveFontData(const char *file)
{
	size_t size;
	FILE *fd = OpenFileSave(file);

	size = fwrite(&jk2font, 1, sizeof(jk2font), fd);

	if (size < sizeof(jk2font)) {
		printf("Short write.\n");
		exit(EXIT_FAILURE);
	}

	fclose(fd);
}

// CSV

#define CSV_DELIMITER_CHAR      '\t'
#define CSV_DELIMITER_STR       "\t"

static char *csvLoadLine;
static char *csvLoadPtr;

void CSVParseLine(const char *line)
{
	if (csvLoadLine) {
		free(csvLoadLine);
	}
	csvLoadLine = strdup(line);
	csvLoadPtr = csvLoadLine;
}

const char *CSVGetString()
{
	static char *field = NULL;

	if (field) {
		free(field);
	}

	if (csvLoadPtr[0] == '\0') {
		return NULL;
	}

	if (csvLoadPtr[0] == '"') {
		csvLoadPtr++;
		const char *ptr = strchr(csvLoadPtr, '"');
		if (ptr == NULL) {
			printf("Invalid CSV.\n");
			exit(EXIT_FAILURE);
		}
		size_t len = ptr - csvLoadPtr;
		field = strndup(csvLoadPtr, len);
		csvLoadPtr += len + 1;
	} else {
		const char *ptr = strchr(csvLoadPtr, CSV_DELIMITER_CHAR);
		if (ptr == NULL) {
			ptr = strchr(csvLoadPtr, '\0');
		}
		size_t len = ptr - csvLoadPtr;
		field = strndup(csvLoadPtr, len);
		csvLoadPtr += len;
	}

	if (csvLoadPtr[0] == CSV_DELIMITER_CHAR) {
		csvLoadPtr++;
	}

	return field;
}

int CSVGetInteger()
{
	return atoi(CSVGetString());
}

float CSVGetFloat()
{
	return atof(CSVGetString());
}

void CSVLoadLine(FILE *fd)
{
	char *line = NULL;
	size_t n = 0;
	ssize_t nread = getline(&line, &n, fd);

	if (nread <= 0) {
		printf("Invalid CSV.\n");
		exit(EXIT_FAILURE);
	}

	CSVParseLine(line);
	free(line);
}

static char *csvSaveLine;

void CSVSaveLine(FILE *fd)
{
	if (csvSaveLine) {
		fputs(csvSaveLine, fd);
		free(csvSaveLine);
		csvSaveLine = NULL;
	}
	fputc('\n', fd);
}

void CSVPutString(const char *string)
{
	if (csvSaveLine) {
		size_t len = strlen(csvSaveLine) + 1 + strlen(string);
		csvSaveLine = realloc(csvSaveLine, len + 1);
		strcat(csvSaveLine, CSV_DELIMITER_STR);
		strcat(csvSaveLine, string);
	} else {
		csvSaveLine = strdup(string);
	}
}

void CSVPutChar(char ch)
{
	char buf[2] = {ch, '\0'};

	CSVPutString(buf);
}

void CSVPutInteger(int i)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%d", i);

	CSVPutString(buf);
}

void CSVPutFloat(float f)
{
	char buf[16];

	snprintf(buf, sizeof(buf), "%.10f", f);

	CSVPutString(buf);
}

void LoadFontDataCSV(const char *file)
{
	FILE *fd = OpenFileLoad(file);

	// font header
	CSVLoadLine(fd);

	// font properties
	CSVLoadLine(fd);
	jk2font.mPointSize = CSVGetInteger();
	jk2font.mHeight = CSVGetInteger();
	jk2font.mAscender = CSVGetInteger();
	jk2font.mDescender = CSVGetInteger();
	jk2font.mKoreanHack = CSVGetInteger();

	// glyp header
	CSVLoadLine(fd);

	for (int i = 0; i < GLYPH_COUNT; i++) {
		glyphInfojk2_t *glyph = &jk2font.mGlyphs[i];

		CSVLoadLine(fd);
		CSVGetInteger(); // ascii code
//		CSVGetString();
		glyph->width = CSVGetInteger();
		glyph->height = CSVGetInteger();
		glyph->horizAdvance = CSVGetInteger();
		glyph->horizOffset = CSVGetInteger();
		glyph->baseline = CSVGetInteger();
		glyph->s = CSVGetFloat();
		glyph->t = CSVGetFloat();
		glyph->s2 = CSVGetFloat();
		glyph->t2 = CSVGetFloat();
	}

	fclose(fd);
}

void SaveFontDataCSV(const char *file)
{
	FILE *fd = OpenFileSave(file);

	// header
	CSVPutString("Point Size");
	CSVPutString("Height");
	CSVPutString("Ascender");
	CSVPutString("Descender");
	CSVPutString("Korean Hack");
	CSVSaveLine(fd);

	CSVPutInteger(jk2font.mPointSize);
	CSVPutInteger(jk2font.mHeight);
	CSVPutInteger(jk2font.mAscender);
	CSVPutInteger(jk2font.mDescender);
	CSVPutInteger(jk2font.mKoreanHack);
	CSVSaveLine(fd);

	// glyph header
	CSVPutString("ASCII");
//	CSVPutString("Character");
	CSVPutString("Width");
	CSVPutString("Height");
	CSVPutString("Horizontal Advance");
	CSVPutString("Horizontal Offset");
	CSVPutString("Baseline");
	CSVPutString("s");
	CSVPutString("t");
	CSVPutString("s2");
	CSVPutString("t2");
	CSVSaveLine(fd);

	for (int i = 0; i < GLYPH_COUNT; i++) {
		glyphInfojk2_t *glyph = &jk2font.mGlyphs[i];

		CSVPutInteger(i);
//		CSVPutChar((char)i);
		CSVPutInteger(glyph->width);
		CSVPutInteger(glyph->height);
		CSVPutInteger(glyph->horizAdvance);
		CSVPutInteger(glyph->horizOffset);
		CSVPutInteger(glyph->baseline);
		CSVPutFloat(glyph->s);
		CSVPutFloat(glyph->t);
		CSVPutFloat(glyph->s2);
		CSVPutFloat(glyph->t2);
		CSVSaveLine(fd);
	}

	fclose(fd);
}

void ConvertToUppercase()
{
	printf("Replacing lowercase glyphs...\n");

	/*
	for (int i = 'a'; i <= 'z'; i++) {
		jk2font.mGlyphs[i] = jk2font.mGlyphs[i - 'a' + 'A'];
	}
	*/
	for (int i = 0xe0; i <= 0xf6; i++) {
		jk2font.mGlyphs[i] = jk2font.mGlyphs[i - 0xe0 + 0xc0];
	}
	for (int i = 0xf8; i <= 0xfe; i++) {
		jk2font.mGlyphs[i] = jk2font.mGlyphs[i - 0xf8 + 0xd8];
	}

	printf("Done\n");
}

void ZeroEmptyGlyphs()
{
	glyphInfojk2_t *glyph;

	printf("Zeroing all fields in empty glyphs...\n");

	for (int i = 0; i <= 0xff; i++) {
		glyph = &jk2font.mGlyphs[i];

		if (glyph->width == 0 && glyph->height == 0 && glyph->horizAdvance == 0) {
			memset(glyph, 0, sizeof(*glyph));
		}
	}

	printf("Done\n");
}

void AdjustErgoeC()
{
	glyphInfojk2_t *glyph;

	printf("Adjusting spacing in Ergoe Condensed Extra Bold font...\n");

	for (int i = 0; i <= 0xff; i++) {
	    glyph = &jk2font.mGlyphs[i];

	    if (glyph->horizAdvance - glyph->width >= 2) {
		// lowercase iso-8859-2 characters
		if ('a' <= i && i <= 'z')
		    glyph->horizAdvance--;

		switch (i) {
		case 177:
		case 179:
		case 181:
		case 182:
		case 185:
		case 186:
		case 187:
		case 188:
		case 190:
		case 191:
		    glyph->horizAdvance--;
		}

		if (224 <= i && i <= 246)
		    glyph->horizAdvance--;
		if (248 <= i && i <= 254)
		    glyph->horizAdvance--;
	    }
	}

	printf("Done\n");
}

void PrintFontData()
{
	glyphInfojk2_t *glyph;
	short height, ascent, descent, offset, width, advance;

	printf("Global parameters:\n\n");
	printf("mPointSize: %d mHeight: %d mAscender: %d mDescender: %d mKoreanHack: %d\n\n",
	       jk2font.mPointSize, jk2font.mHeight, jk2font.mAscender, jk2font.mDescender, jk2font.mKoreanHack);

	printf("Glyph parameters:\n\n");

	for (int i = 0; i < GLYPH_COUNT; i++) {
		glyph = &jk2font.mGlyphs[i];

		height = glyph->height;
		ascent = glyph->baseline;
		descent = height - glyph->baseline;
		offset = glyph->horizOffset;
		width = glyph->width;
		advance = glyph->horizAdvance;

		printf("0x%02x ", i);

		if (isprint(i))
			printf("'%c'", i);
		else
			printf("   ");

		printf("   Height: %3d Ascent: %3d Descent: %3d         Offset: %3d Width: %3d Advance: %3d\n",
		       height, ascent, descent, offset, width, advance );
	}
}

#define FLAG_UPPERCASE 0x01
#define FLAG_ZERO      0x02
#define FLAG_ERGOEC    0x04
#define FLAG_IMPORT    0x08
#define FLAG_EXPORT    0x10
#define FLAG_PRINT     0x20

int main (int argc, char *argv[])
{
	const char *file;
	char *filecsv;
	int opt;
	int flags = 0;

	progName = argv[0];

	while ((opt = getopt(argc, argv, "uzciep")) != -1) {
		switch (opt) {
		case 'u':
			flags |= FLAG_UPPERCASE;
			break;
		case 'z':
			flags |= FLAG_ZERO;
			break;
		case 'c':
			flags |= FLAG_ERGOEC;
			break;
		case 'i':
			flags |= FLAG_IMPORT;
			break;
		case 'e':
			flags |= FLAG_EXPORT;
			break;
		case 'p':
			flags |= FLAG_PRINT;
			break;
		case '?':
			PrintHelpExit(EXIT_SUCCESS);
		}
	}

	if (optind >= argc) {
		PrintHelpExit(EXIT_FAILURE);
	}

	file = argv[optind];

	if (file[0] == '\0') {
		PrintHelpExit(EXIT_FAILURE);
	}

	filecsv = malloc(strlen(file) + strlen(".csv") + 1);
	strcpy(filecsv, file);
	strcat(filecsv, ".csv");

	if (flags & FLAG_IMPORT) {
		LoadFontDataCSV(filecsv);
	} else {
		LoadFontData(file);
	}

	if (flags & FLAG_UPPERCASE) {
		ConvertToUppercase();
	}
	if (flags & FLAG_ZERO) {
		ZeroEmptyGlyphs();
	}
	if (flags & FLAG_ERGOEC) {
		AdjustErgoeC();
	}
	if (flags & FLAG_PRINT) {
		PrintFontData();
		exit(EXIT_SUCCESS);
	}

	if (flags & FLAG_EXPORT) {
		SaveFontDataCSV(filecsv);
	} else {
		SaveFontData(file);
	}

	return 0;
}
