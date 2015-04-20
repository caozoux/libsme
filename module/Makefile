
TARGET :=
SUBDIR := buffer notify
#SUBDIR_CLEAN := buffer_clean notify_clean
SUBDIR_CLEAN := $(patsubst %_clean,%,$(SUBDIR))
.PHONY : $(SUBDIR) $(SUBDIR_CLEAN)
ALL: $(SUBDIR)

$(SUBDIR): 
	$(Q)$*(MAKE) -C $@

clean: $(SUBDIR_CLEAN) 

$(SUBDIR_CLEAN):
	$(Q)$(MAKE) -C $(subst _clean,,$@) clean
