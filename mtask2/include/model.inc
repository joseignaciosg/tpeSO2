ifdef __HUGE__
	.model huge
	FAR_DATA = 1
	FAR_CODE = 1
else
ifdef __LARGE__
	.model large
	FAR_DATA = 1
	FAR_CODE = 1
else
ifdef __COMPACT__
	.model compact
	FAR_DATA = 1
	FAR_CODE = 0
else
ifdef __MEDIUM__
	.model medium
	FAR_DATA = 0
	FAR_CODE = 1
else
ifdef __SMALL__
	.model small
	FAR_DATA = 0
	FAR_CODE = 0
else
ifdef __TINY__
	.model tiny
	FAR_DATA = 0
	FAR_CODE = 0
else
	%out Error: Modelo de memoria no especificado
	.err
endif
endif
endif
endif
endif
endif
