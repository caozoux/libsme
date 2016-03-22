/*
  Author : Andrew Huang <bluedrum@163.com>
   <<Care and feeding of your Human Interface Devices>>
   http://lxr.free-electrons.com/source/Documentation/usb/hiddev.txt?v=2.6.30
   http://www.wetlogic.net/hiddev/
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/hiddev.h>

static void showReports(int fd, unsigned report_type);
static void show_all_report(int fd);
static int read_event(int fd);

int main()
{
  char dev_name[64] = "/dev/hiddev0";
  int fd = -1;
  int version;
    char name[100];
    struct hiddev_devinfo dinfo;
  
  
  fd = open(dev_name,O_RDWR);
  if(fd == -1)
    {
       fprintf(stderr,"open %s failure\n",dev_name);
       return -1;
    }
    
    
   printf("%s infor\n",dev_name); 
  
       if (ioctl(fd, HIDIOCGVERSION, &version) < 0)
            perror("HIDIOCGVERSION");
        else
        {
            printf("HIDIOCGVERSION: %d.%d\n", (version>>16) & 0xFFFF, version & 0xFFFF);
            if (version != HID_VERSION)
                printf("WARNING: version does not match compile-time version\n");
        }
        
        if (ioctl(fd, HIDIOCGDEVINFO, &dinfo) < 0)
            perror("HIDIOCGDEVINFO");
        else
        {
            printf("HIDIOCGDEVINFO: bustype=%d busnum=%d devnum=%d ifnum=%d\n"
                "\tvendor=0x%04hx product=0x%04hx version=0x%04hx\n"
                "\tnum_applications=%d\n",
                dinfo.bustype, dinfo.busnum, dinfo.devnum, dinfo.ifnum,
                dinfo.vendor, dinfo.product, dinfo.version, dinfo.num_applications);
        }
        
            if (ioctl(fd, HIDIOCGNAME(99), name) < 0)
            perror("HIDIOCGNAME");
        else
        {
            name[99] = 0;
            printf("HIDIOCGNAME: %s\n", name);
        }
        
    #if 0    
        printf("\n*** INPUT:\n"); showReports(fd, HID_REPORT_TYPE_INPUT);
            printf("\n*** OUTPUT:\n"); showReports(fd, HID_REPORT_TYPE_OUTPUT);
            printf("\n*** FEATURE:\n"); showReports(fd, HID_REPORT_TYPE_FEATURE);
  #endif
    show_all_report(fd);    

    read_event(fd);
        
        close(fd);
        
        
  
}




static void showReports(int fd, unsigned report_type)
{
    struct hiddev_report_info rinfo;
    struct hiddev_field_info finfo;
    struct hiddev_usage_ref uref;
    int i, j, ret;

    rinfo.report_type = report_type;
    rinfo.report_id = HID_REPORT_ID_FIRST;
    ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
    while (ret >= 0)
    {
        printf("HIDIOCGREPORTINFO: report_id=0x%X (%u fields)\n",
            rinfo.report_id, rinfo.num_fields);
        for (i = 0; i < rinfo.num_fields; i++)
        {
            finfo.report_type = rinfo.report_type;
            finfo.report_id = rinfo.report_id;
            finfo.field_index = i;
            ioctl(fd, HIDIOCGFIELDINFO, &finfo);

            printf("HIDIOCGFIELDINFO: field_index=%u maxusage=%u flags=0x%X\n"
                "\tphysical=0x%X logical=0x%X application=0x%X\n"
                "\tlogical_minimum=%d,maximum=%d physical_minimum=%d,maximum=%d\n",
                finfo.field_index, finfo.maxusage, finfo.flags,
                finfo.physical, finfo.logical, finfo.application,
                finfo.logical_minimum, finfo.logical_maximum,
                finfo.physical_minimum, finfo.physical_maximum);

            for (j = 0; j < finfo.maxusage; j++)
            {
                uref.report_type = finfo.report_type;
                uref.report_id = finfo.report_id;
                uref.field_index = i;
                uref.usage_index = j;
                ioctl(fd, HIDIOCGUCODE, &uref);
                ioctl(fd, HIDIOCGUSAGE, &uref);

                printf(" >> usage_index=%u usage_code=0x%X () value=%d\n",
                    uref.usage_index,
                    uref.usage_code,
                //    controlName(uref.usage_code),
                    uref.value);

            }
        }
        printf("\n");

        rinfo.report_id |= HID_REPORT_ID_NEXT;
        ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
    }
}


void show_all_report(int fd)
{
    
    struct hiddev_report_info rinfo;
    struct hiddev_field_info finfo;
    struct hiddev_usage_ref uref;
    int rtype, i, j;
    char *rtype_str;

    for (rtype = HID_REPORT_TYPE_MIN; rtype <= HID_REPORT_TYPE_MAX;
     rtype++) {
     switch (rtype) {
        case HID_REPORT_TYPE_INPUT: rtype_str = "Input"; break;
        case HID_REPORT_TYPE_OUTPUT: rtype_str = "Output"; break;
        case HID_REPORT_TYPE_FEATURE: rtype_str = "Feature"; break;
        default: rtype_str = "Unknown"; break;
     }
     fprintf(stdout, "Reports of type %s (%d):\n", rtype_str, rtype);
     rinfo.report_type = rtype;
     rinfo.report_id = HID_REPORT_ID_FIRST;
     while (ioctl(fd, HIDIOCGREPORTINFO, &rinfo) >= 0) {
        fprintf(stdout, " Report id: %d (%d fields)\n",
         rinfo.report_id, rinfo.num_fields);
        for (i = 0; i < rinfo.num_fields; i++) { 
         memset(&finfo, 0, sizeof(finfo));
         finfo.report_type = rinfo.report_type;
         finfo.report_id = rinfo.report_id;
         finfo.field_index = i;
         ioctl(fd, HIDIOCGFIELDINFO, &finfo);
         fprintf(stdout, " Field: %d: app: %04x phys %04x "
             "flags %x (%d usages) unit %x exp %d\n", 
             i, finfo.application, finfo.physical, finfo.flags,
             finfo.maxusage, finfo.unit, finfo.unit_exponent);
         memset(&uref, 0, sizeof(uref));
         for (j = 0; j < finfo.maxusage; j++) {
            uref.report_type = finfo.report_type;
            uref.report_id = finfo.report_id;
            uref.field_index = i;
            uref.usage_index = j;
            ioctl(fd, HIDIOCGUCODE, &uref);
            ioctl(fd, HIDIOCGUSAGE, &uref);
            fprintf(stdout, " Usage: %04x val %d\n", 
                uref.usage_code, uref.value);
         }
        }
        rinfo.report_id |= HID_REPORT_ID_NEXT;
     }
    }
    // if (!run_as_daemon)
     fprintf(stdout, "Waiting for events ... (interrupt to exit)\n");
    
}

int read_event(int fd)
{
     struct hiddev_event ev[64];
     int i,rd;
     time_t curr_time;
     char name[100];
     
     while(1)
     {
     rd = read(fd, ev, sizeof(ev));
     if (rd < (int) sizeof(ev[0])) {
        if (rd < 0)
         perror("\nevtest: error reading");
    
         return -1;
     }
     
     for (i = 0; i < rd / sizeof(ev[0]); i++) {
         //int idx = info_idx(ev[i].hid);
         int idx = ev[i].hid;
         curr_time = time(NULL);
         strftime(name, sizeof(name), "%b %d %T", 
            localtime(&curr_time));
         fprintf(stdout, "%s: Event: usage %x ( ), value %d\n",
             name, ev[i].hid, 
             /* (idx >= 0) ? ups_info[idx].label : "Unknown",*/
             ev[i].value);
        }

  }
  return 0;
}
