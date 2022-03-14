#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#define MAX 10

typedef unsigned char byte;

int ok = 0;
char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
byte hash1[MD5_DIGEST_LENGTH]; // password hash (target)
byte hash2[MD5_DIGEST_LENGTH]; // string hashes
char hash1_str[2 * MD5_DIGEST_LENGTH + 1];

struct thread_args
{
	int id;
	char prefix[37];
	char pwd[11];
};

int crack_password_helper2(char *pwd_pred, int pwd_len, int keysize)
{
	if (ok == 1)
		return 1;
	else if (pwd_len >= keysize)
	{
		MD5((byte *)pwd_pred, strlen(pwd_pred), hash2);
		if (strncmp((char *)hash1, (char *)hash2, MD5_DIGEST_LENGTH) == 0)
		{
			printf("found: %s\n", pwd_pred);
			// print_digest(hash2);
			ok = 1;
			return 1;
		} else 
			return -1;
	}

	for (int i = 0; i < 36; i++)
	{
		pwd_pred[pwd_len] = letters[i];
		int ret = crack_password_helper2(pwd_pred, pwd_len + 1, keysize); // explore

		if (ret == 1)
			return 1;
	}
	return -1;
}

/*
 * This procedure generate all combinations of possible letters
*/
void crack_password_helper(char *pwd_pred, int idx, int len)
{
	int c;
	// printf("%s\n", pwd_pred);
	if (ok == 1)
		return;
	if (idx < len - 1)
	{
		for (c = 0; c < 36 && ok == 0; c++)
		{
			pwd_pred[idx] = letters[c];
			crack_password_helper(pwd_pred, idx + 1, len);
		}
	}
	else
	{
		for (c = 0; c < 36 && ok == 0; c++)
		{
			pwd_pred[idx] = letters[c];
			// printf("pwd-pred: %s, strlen: %d, idx: %d, len: %d\n", pwd_pred, strlen(pwd_pred), idx, len);
			MD5((byte *)pwd_pred, strlen(pwd_pred), hash2);
			if (strncmp((char *)hash1, (char *)hash2, MD5_DIGEST_LENGTH) == 0 && ok == 0)
			{
				printf("found: %s\n", pwd_pred);
				ok = 1;
				break;
			}
		}
	}
}

void *trampoline(void *args)
{
	struct thread_args *input = (struct thread_args *)args;
	// printf("prefix %s for id %d\n", input->prefix, input->id);

	for (int i = 0; i < strlen(input->prefix); i++)
	{
		if (ok == 1)
			break;
		
		for (int j = 1; j <= MAX; j++)
		{
			memset(input->pwd, 0, j+1);
			input->pwd[0] = input->prefix[i];
			
			crack_password_helper(input->pwd, 1, j);
			// crack_password(input->pwd, j);
			if (ok == 1)
				break;
		}
	}
	return NULL;
}

/*
 * Convert hexadecimal string to hash byte.
*/
void strHex_to_byte(char *str, byte *hash)
{
	char *pos = str;
	int i;

	for (i = 0; i < MD5_DIGEST_LENGTH / sizeof *hash; i++)
	{
		sscanf(pos, "%2hhx", &hash[i]);
		pos += 2;
	}
}

/*
 * Print a digest of MD5 hash.
*/
void print_digest(byte *hash)
{
	int x;

	for (x = 0; x < MD5_DIGEST_LENGTH; x++)
		printf("%02x", hash[x]);
	printf("\n");
}

int main(int argc, char **argv)
{
	// Determine the amount of available CPUs
	int cpus = get_nprocs();
	// nprocs() might return wrong amount inside of a container.
	// Use MAX_CPUS instead, if available.
	if (getenv("MAX_CPUS"))
	{
		cpus = atoi(getenv("MAX_CPUS"));
	}
	// Sanity-check
	assert(cpus > 0 && cpus <= 64);

	struct thread_args args_thread[cpus];
	pthread_t thread[cpus];

	int r;

	// Input:
	r = scanf("%s", hash1_str);

	// Check input.
	if (r == EOF || r == 0)
	{
		fprintf(stderr, "Error!\n");
		exit(1);
	}

	// Convert hexadecimal string to hash byte.
	strHex_to_byte(hash1_str, hash1);
	memset(hash2, 0, MD5_DIGEST_LENGTH);

	int work_size = work_divider(cpus);
	if (work_size == 0)
		work_size = 1;

	for (int i = 0; i < cpus; i++)
	{
		args_thread[i].id = i;

		if (i < cpus - 1)
			substring(args_thread[i].prefix, i * work_size, (i + 1) * work_size - 1);
		else
			substring_l(args_thread[i].prefix, i * work_size);

		pthread_create(&thread[i], NULL, trampoline, &args_thread[i]);
		if (ok == 1)
			break;
	}

	for (int i = 0; i < cpus; i++)
	{
		pthread_join(thread[i], NULL);
	}
}

int work_divider(int num_threads)
{
	return 36 / num_threads;
}

void substring(char *buffer, int left_index, int right_index)
{
	char substr[36];
	int index = 0;

	for (int i = left_index; i <= right_index; i++)
	{
		substr[index] = letters[i];
		index++;
	}

	substr[index] = '\0';
	strcpy(buffer, substr);
}

void substring_l(char *buffer, int left_index)
{
	char substr[36];
	int index = 0;

	for (int i = left_index; i < strlen(letters); i++)
	{
		substr[index] = letters[i];
		index++;
	}

	substr[index] = '\0';
	strcpy(buffer, substr);
}
