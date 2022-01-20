#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "user.h"

static void clear_new_line(char *str)
{
	for (int i = 0; str[i] != '\0'; ++i){
		if (str[i] == '\n'){
			str[i] = '\0';
			break;
		}
	}
}

int save_users(const char *filename, List *users)
{
	if (!filename || !users)
		return 1;

#if 0
	char postfix[] = ".tmp";
	int plen = strlen(postfix);
	int flen = strlen(filename);
	char tmpfile[flen+plen+1];
	memset(tmpfile, 0, flen+plen+1);
	strncpy(tmpfile, filename, flen);
	strncat(tmpfile, postfix, plen);

	printf("%s\n", tmpfile);
	rename(filename, tmpfile);
#endif
	FILE *fp;
	fp = fopen(filename, "w");
	if (!fp){
		perror(filename);
		return 1;
	}

	ListElmt 	*elmt;
	User 		*user;
	for (elmt = list_head(users); elmt != NULL; elmt = list_next(elmt))
	{
		user = (User *)list_data(elmt);
		printf("%s\n", user->username);
		printf("%s\n", user->password);
		if (fprintf(fp, "%s\n", user->username) == 0){
			fprintf(stderr, "Error to save users\n");
			return 1;
		}
		if (fprintf(fp, "%s\n", user->password) == 0){
			fprintf(stderr, "Error to save users\n");
			return 1;
		}
	}
	fclose(fp);

#if 0
	remove(tmpfile);
#endif

	return 0;
}

int load_users(const char *filename, List *users)
{
	if (!filename || !users)
		return 1;

	FILE *fp;
	fp = fopen(filename, "r");
	if (!fp){
		if (errno == ENOENT){
			fopen(filename, "w");
			fclose(fp);
			return 0;
		}
		perror(filename);
		return 1;
	}

	while(!feof(fp)){
		User *user = (User *)malloc(sizeof(User));
		if (!user){
			perror("malloc()");
			fclose(fp);
			return 1;
		}
		memset(user, 0, sizeof(User));

		if (fscanf(fp, "%s%s", user->username, user->password) != 2){
			free(user);
			break;
		}
		list_ins_next(users, NULL, (void *)user);
	}

	fclose(fp);
	return 0;
}

User *user_search_by_username(List *users, const char *username)
{
	if (!users || !username)
		return NULL;
	ListElmt *elm;
	User     *user;
	for (elm = list_head(users); elm != NULL; elm = list_next(elm)) {
		user = ((User *)list_data(elm));
		if (strncmp(user->username, username, USERNAME_LEN) == 0)
			return user;
	}
	return NULL;
}

void user_print(List *users)
{
	if (!users)
		return;

	ListElmt *elm;
	for (elm = list_head(users); elm != NULL; elm = list_next(elm)){
		printf("Username: %s\n", ((User *)list_data(elm))->username);
		printf("Password: %s\n", ((User *)list_data(elm))->password);
		printf("-----------------------\n");
	}
}

void user_destroy(void *data)
{
	free(data);
}
