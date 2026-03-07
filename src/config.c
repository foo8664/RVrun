#include "config.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "debug.h"

static struct rvconfig *_rvglobalcfg = NULL;

static char *smemcpy(const char *src) __attribute__((nonnull, malloc(free)));
static void freeopt(struct optpair *pair, configclean_t cleanup)
	__attribute__((nonnull(1)));

void cleancfg(struct rvconfig *cfg, configclean_t cleanup)
{
	size_t i;

	for (i = 0; i < cfg->size; ++i)
		freeopt(&cfg->opts[i], cleanup);
}

void parsecfg(struct rvconfig *cfg, int argc, char **argv)
{
	struct option *longopts;
	size_t i;
	int ret;
	int index;

	if (!(longopts = malloc((cfg->size + 1) * sizeof(*longopts))))
		panic("malloc failed");

	for (i = 0; i < cfg->size; ++i) {
		cfg->opts[i].rvopt.set = false;
		cfg->opts[i].rvopt.arg = false;
		cfg->opts[i].opt.val = 0;
		longopts[i] = cfg->opts[i].opt;
	}
	longopts[i] = (struct option){0};

	while (!(ret = getopt_long_only(argc, argv, cfg->optstring, longopts,
					&index))) {

		cfg->opts[index].rvopt.set = true;
		if (cfg->opts[index].opt.has_arg == NO_ARG)
			continue;

		switch (cfg->opts[index].rvopt.type) {
		case SET_STDIN:
			assert(cfg->opts[index].opt.has_arg == ARG_MANDATORY);
			cfg->opts[index].rvopt.u.str = smemcpy(optarg);
			cfg->opts[index].rvopt.arg = true;
			assert(cfg->opts[index].rvopt.u.str);
			break;
		}
	}

	free(longopts);
	if (ret != -1)
		panic("Invalid option");
}

struct rvopt *getcfgopt(struct rvconfig *cfg, enum RVOPT type)
{
	size_t i;

	for (i = 0; i < cfg->size; ++i)
		if (cfg->opts[i].rvopt.set && cfg->opts[i].rvopt.type == type)
			return &cfg->opts[i].rvopt;
	return NULL;
}

void set_globalcfg(struct rvconfig *cfg)
{
	_rvglobalcfg = cfg;
}

struct rvconfig *get_globalcfg(void)
{
	return _rvglobalcfg;
}


static char *smemcpy(const char *src)
{
	size_t n;
	char *dst;

	n = strlen(src);
	if (!(dst = malloc((n + 1) * sizeof(*dst))))
		return NULL;

	return strncpy(dst, src, n);
}

static void freeopt(struct optpair *pair, configclean_t cleanup)
{
	if (cleanup)
		cleanup(pair);

	switch (pair->rvopt.type) {
	case SET_STDIN:
		free(pair->rvopt.u.str);
		break;
	}
}
