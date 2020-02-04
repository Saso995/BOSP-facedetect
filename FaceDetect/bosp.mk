
ifdef CONFIG_CONTRIB_FACEDETECT

# Targets provided by this project
.PHONY: facedetect clean_facedetect

# Add this to the "contrib_testing" target
testing: facedetect
clean_testing: clean_facedetect

MODULE_CONTRIB_USER_FACEDETECT=contrib/user/FaceDetect

facedetect: external
	@echo
	@echo "==== Building FaceDetect ($(BUILD_TYPE)) ===="
	@echo " Using GCC    : $(CC)"
	@echo " Target flags : $(TARGET_FLAGS)"
	@echo " Sysroot      : $(PLATFORM_SYSROOT)"
	@echo " BOSP Options : $(CMAKE_COMMON_OPTIONS)"
	@[ -d $(MODULE_CONTRIB_USER_FACEDETECT)/build/$(BUILD_TYPE) ] || \
		mkdir -p $(MODULE_CONTRIB_USER_FACEDETECT)/build/$(BUILD_TYPE) || \
		exit 1
	@cd $(MODULE_CONTRIB_USER_FACEDETECT)/build/$(BUILD_TYPE) && \
		CC=$(CC) CFLAGS="$(TARGET_FLAGS)" \
		CXX=$(CXX) CXXFLAGS="$(TARGET_FLAGS)" \
		cmake $(CMAKE_COMMON_OPTIONS) ../.. || \
		exit 1
	@cd $(MODULE_CONTRIB_USER_FACEDETECT)/build/$(BUILD_TYPE) && \
		make -j$(CPUS) install || \
		exit 1

clean_facedetect:
	@echo
	@echo "==== Clean-up FaceDetect Application ===="
	@[ ! -f $(BUILD_DIR)/usr/bin/facedetect ] || \
		rm -f $(BUILD_DIR)/etc/bbque/recipes/FaceDetect*; \
		rm -f $(BUILD_DIR)/usr/bin/facedetect*
	@rm -rf $(MODULE_CONTRIB_USER_FACEDETECT)/build
	@echo

else # CONFIG_CONTRIB_FACEDETECT

facedetect:
	$(warning contib module FaceDetect disabled by BOSP configuration)
	$(error BOSP compilation failed)

endif # CONFIG_CONTRIB_FACEDETECT

