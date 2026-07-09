SHELL = /bin/bash

.PHONY : clean all
all : $(TARGET_A) $(TARGET_SO)

%.o: %.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

%.o: %.S
	$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS) $(OBJS_ASM)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS) $(OBJS_ASM)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))
	@echo "$$AR_MRI" | $(AR) -M

$(TARGET_SO): $(OBJS) $(OBJS_ASM)
	@$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(TARGET_SO) -Wl,--start-group $(OBJS) $(OBJS_ASM) $(LIBS) -Wl,--end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

clean:
	@rm -f $(OBJS) $(OBJS_ASM) $(DEPS) $(DEPS_ASM) $(TARGET_A) $(TARGET_SO)

-include $(DEPS)
