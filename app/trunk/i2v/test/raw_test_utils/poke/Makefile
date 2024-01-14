prefix=/opt/mtwk/usr/local/powerpc-603e-linux/gcc-3.3.2-glibc-2.3.2
cross_compile=powerpc-603e-linux-
PATH:=${prefix}/bin:${PATH};
ifeq ($(GCC),)
export GCC=${cross_compile}gcc
export STRIP=${cross_compile}strip
endif
ifeq ($(BUILD_DIR),)
export BUILD_DIR=/tftpboot
endif
export CONFIG_DIR=$(BUILD_DIR)
export LIB_DIR=$(BUILD_DIR)/usr/lib

export I2V_DIR=../../..
export I2V_INC=-I${I2V_DIR}/common
export WSU_DIR=../../../../WSUServices
export WSU_API_DIR=$(WSU_DIR)/wsu_services_api
export WSU_API_INC=-I${WSU_API_DIR}/include
export WSU_INC_DIR=${WSU_DIR}/Common/wsu_include
export WSU_COMMON_INC=-I${WSU_INC_DIR} -I${WSU_INC_DIR}/shm_inc
export WSU_API_LIB=${WSU_API_DIR}/lib
export SMI_LIB_PATH=${WSU_DIR}/esbsmi

export I2V_LIB=$(LIB_DIR)/libi2v.so
export I2V_LIB_COMPILE=-L${SMI_LIB_PATH} -lesbsmi -L${WSU_API_LIB} -lrissmi -ltpsapi

export POKE_INCLUDES=${WSU_API_INC} ${WSU_COMMON_INC} ${I2V_INC}
export POKE_LIBRARIES=-L${LIB_DIR} -li2v ${I2V_LIB_COMPILE} -lipc

export POKE_SOURCE=poke.c

.PHONY: all deploy

all: prereq poke deploy

prereq:
	bash -c 'if [ ! -e ${I2V_LIB} ]; then echo "required i2v library missing - build code first"; exit 1; fi'

poke:
	${GCC} -Wall -W -g -O2 ${POKE_INCLUDES} ${POKE_LIBRARIES} ${POKE_SOURCE} -o ${BUILD_DIR}/poke
	${STRIP} ${BUILD_DIR}/poke

deploy:
	@echo "for now nothing else needed - build completed"
