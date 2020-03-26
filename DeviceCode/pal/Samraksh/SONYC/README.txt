Based on X-CUBE-AI 5.0.0 from ST and compiled with GCC 6.3.1

In order to generate libsonyc_ml.a a CubeMX project was created and the following was added to the makefile:

$(BUILD_DIR)/libsonyc_ml.a: $(BUILD_DIR)/network.o $(BUILD_DIR)/network_data.o $(BUILD_DIR)/app_x-cube-ai.o
	$(AR) -cvr $(BUILD_DIR)/lib_ml_temp.a $^
	$(AR) -M <build-lib.mri


Contents of build-lib.mri:

create build/libsonyc_ml.a
addlib build/lib_ml_temp.a
addlib Middlewares/ST/AI/Lib/NetworkRuntime500_CM7_GCC.a
save
end

Note that NetworkRuntime500_CM7_GCC.a is linked in so there is a hard dependency on X-CUBE-AI 5.0.0 and Cortex M7.
In the event of a version upgrade or MCU change, you must rebuild the library from a fresh CubeMX project.
ST distributes everything for free so this should pose no issue.

One modification is made to the X-CUBE-AI code (specifically, app_x-cube-ai.c), the large-ish activation memory is given attribute:
static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE] __attribute__ (( section (".ram_d1") ));

This is specific to the STM32H743/753. This section must be defined in eMote or otherwise accounted for. Does not exist by default.
This change is done because 'activations' won't fit in the primary memory area (DTCM).

Additionally, a suitable math lib must be provided, e.g., libarm_cortexM7lfdp_math.a
Typically this would also be pulled from the CubeMX project

Header files also must be copied over manually.

Nathan Stohs
nathan.stohs@samraksh.com
2020-02-12

Version History:

2020-02-12
1a07e521bc226bcbaa5cf38e9c37cb4f *build/libsonyc_ml.a
Initial

2020-03-26
9c9b251bf3bb030211f895449a258810 *libsonyc_ml.a
New split-model (2) architecture
Proper quantized learning
