#include "config.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "debug.h"

static struct rvconfig *_rvglobalcfg = NULL;

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
		case CFG_STDIN:
			assert(cfg->opts[index].opt.has_arg == ARG_MANDATORY);

			cfg->opts[index].rvopt.arg = true;
			cfg->opts[index].rvopt.u.integer = open(optarg, O_RDONLY);
			if (cfg->opts[index].rvopt.u.integer < 0) {
				err_log("%s: %s", optarg, strerror(errno));
				panic("Could not open file");
			}
			break;

		case CFG_STDOUT:
		case CFG_STDERR:
			assert(cfg->opts[index].opt.has_arg == ARG_MANDATORY);

			cfg->opts[index].rvopt.arg = true;
			cfg->opts[index].rvopt.u.integer = open(optarg,
					O_WRONLY | O_CREAT | O_TRUNC, 0777);
			if (cfg->opts[index].rvopt.u.integer < 0) {
				err_log("%s: %s", optarg, strerror(errno));
				panic("Could not open file");
			}
			break;

		case CFG_LOGFILE:
			assert(cfg->opts[index].opt.has_arg == ARG_MANDATORY);

			cfg->opts[index].rvopt.arg = true;
			cfg->opts[index].rvopt.u.ptr = (void *)fopen(optarg, "w");
			if (!cfg->opts[index].rvopt.u.ptr) {
				err_log("%s: %s", optarg, strerror(errno));
				panic("Could not open file");
			}
			break;

		case CFG_LOGLEVEL:
			assert(cfg->opts[index].opt.has_arg == ARG_MANDATORY);

			cfg->opts[index].rvopt.arg = true;
			if (sscanf(optarg, "%u", &cfg->opts[index].rvopt.u.uint) != 1) {
				err_log("%s: %s", optarg, strerror(errno));
				panic("Could not get log-level");
			}

			if (cfg->opts[index].rvopt.u.uint > ERR_LOGLEVEL) {
				warn_log("Invalid log level %d, Reducing to %d",
					 cfg->opts[index].rvopt.u.uint,
					 ERR_LOGLEVEL);
				cfg->opts[index].rvopt.u.uint = ERR_LOGLEVEL;
			}

			break;

		default:
			panic("Invalid option type");
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


static void freeopt(struct optpair *pair, configclean_t cleanup)
{
	if (!pair->rvopt.set)
		return;

	if (cleanup)
		cleanup(pair);

	switch (pair->rvopt.type) {
	case CFG_LOGFILE:
		fclose((FILE *)pair->rvopt.u.ptr);
		break;

	case CFG_STDIN:
	case CFG_STDOUT:
	case CFG_STDERR:
		close(pair->rvopt.u.integer);
		break;

	default:
		break;
	}
}
