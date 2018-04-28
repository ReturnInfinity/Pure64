#include <pure64/uuid.h>

#include <pure64/string.h>

static char pure64_tolower(char c) {

	if ((c >= 'A') && (c <= 'Z'))
		c = 'a' + (c - 'A');

	return c;
}

void pure64_uuid_zero(struct pure64_uuid *uuid) {
	pure64_memset(uuid->bytes, 0, sizeof(uuid->bytes));
}

void pure64_uuid_copy(struct pure64_uuid *dst,
                      const struct pure64_uuid *src) {
	pure64_memcpy(dst->bytes, src->bytes, sizeof(dst->bytes));
}

int pure64_uuid_cmp(const struct pure64_uuid *a,
                    const struct pure64_uuid *b) {

	return pure64_memcmp(a->bytes, b->bytes, sizeof(a->bytes));
}

int pure64_uuid_parse(struct pure64_uuid *uuid, const char *str) {

	unsigned int i;
	unsigned int j;

	i = 0;
	j = 0;

	while ((i < 16) && (str[j] != 0)) {

		uuid->bytes[i] = 0;

		char c = pure64_tolower(str[j]);
		if ((c >= '0') && (c <= '9'))
			uuid->bytes[i] |= ((c - '0') + 0x00) << 4;
		else if ((c >= 'a') && (c <= 'f'))
			uuid->bytes[i] |= ((c - 'a') + 0x0a) << 4;
		else if (c == '-') {
			j++;
			continue;
		} else
			return -1;

		c = pure64_tolower(str[j + 1]);
		if ((c >= '0') && (c <= '9'))
			uuid->bytes[i] |= (c - '0') + 0x00;
		else if ((c >= 'a') && (c <= 'f'))
			uuid->bytes[i] |= (c - 'a') + 0x0a;
		else
			return -1;

		j += 2;
		i++;
	}

	/* adjust for little endian encoding */

	unsigned char buf[16];

	/* little endian */

	buf[0] = uuid->bytes[3];
	buf[1] = uuid->bytes[2];
	buf[2] = uuid->bytes[1];
	buf[3] = uuid->bytes[0];

	/* little endian */

	buf[4] = uuid->bytes[5];
	buf[5] = uuid->bytes[4];

	/* little endian */

	buf[6] = uuid->bytes[7];
	buf[7] = uuid->bytes[6];

	/* big endian */

	buf[8] = uuid->bytes[8];
	buf[9] = uuid->bytes[9];

	/* big endian */

	buf[10] = uuid->bytes[10];
	buf[11] = uuid->bytes[11];
	buf[12] = uuid->bytes[12];
	buf[13] = uuid->bytes[13];
	buf[14] = uuid->bytes[14];
	buf[15] = uuid->bytes[15];

	for (i = 0; i < 16; i++)
		uuid->bytes[i] = buf[i];

	return 0;
}
