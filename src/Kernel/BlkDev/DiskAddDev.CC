U0 BlkDevLockFwdingSet(CBlkDev *bd)
{  //If two blkdevs on same controller, use just one lock
	CBlkDev *bd1;
	I64 i;
	switch (bd->type)
	{
		case BDT_RAM:
			break;
		case BDT_ISO_FILE_READ:
		case BDT_ISO_FILE_WRITE:
			bd->lock_fwding = Letter2BlkDev(*bd->file_disk_name);
			break;
		case BDT_ATA:
		case BDT_ATAPI:
			for (i = 0; i < BLKDEVS_NUM; i++)
			{
				bd1 = &blkdev.blkdevs[i];
				if (bd1->bd_signature == BD_SIGNATURE_VAL && bd != bd1 &&
					(bd1->type == BDT_ATAPI || bd1->type == BDT_ATA) &&
					bd1->base0 == bd->base0)
				{
					bd->lock_fwding = bd1;
					break;
				}
			}
			break;
	}
}

I64 BlkDevAdd(CBlkDev *bd, I64 prt_num=I64_MIN, Bool whole_drive, Bool make_free)
{//It will mount just one partition of prt_num>=0.
//When repartitioing whole drive, whole_drive=TRUE.
	I64 i, j, ext_base, offset, res = 0, num = 0;
	CDrive *drive;
	CRedSeaBoot br;
	CMasterBoot mbr;

	bd->bd_signature = BD_SIGNATURE_VAL;
	if (make_free)
		drive = DriveMakeFreeSlot(bd->first_drive_let);
	else
		drive = DriveMakeFreeSlot(DriveNextFreeLet(bd->first_drive_let));
	drive->bd = bd;
	drive->drv_offset = bd->drv_offset;
	drive->size = bd->max_blk + 1 - bd->drv_offset;
	switch (bd->type)
	{
		case BDT_RAM:
		case BDT_ISO_FILE_READ:
		case BDT_ISO_FILE_WRITE:
			drive->drive_signature = DRIVE_SIGNATURE_VAL;
			drive->prt_num = num;
			drive->fs_type = FSt_REDSEA;
			//This is to force creation of a RAM
			//drive during boot, so it is probably
			//MAlloced to the same addr and can
			//be assumed to be already formatted.
			//If this line is removed, RAM Drives
			//will be alloced on a just-in-time
			//basis.
			if (BlkDevInit(bd))
				res++;
			else
				drive->drive_signature = 0;
			break;
		case BDT_ATA:
			drive->drive_signature = DRIVE_SIGNATURE_VAL; //Temporarily validate
			if (!BlkDevInit(bd)) {
				drive->drive_signature = 0; //Revoke validation
				break;
			}
			//FALLTHRU
		case BDT_INT13H:
				drive->drive_signature = 0; //Revoke validation
				if (whole_drive)
				{
					drive->drive_signature = DRIVE_SIGNATURE_VAL;
					drive->prt_num = num;
					res++;
					drive->fs_type = FSt_REDSEA;
					drive->size = bd->max_blk + 1 - bd->drv_offset;
					//The following read is a test read.
					//if it hangs, the drive is not supported.
					if (bd->type == BDT_ATA)
						ATAReadBlks(bd, &mbr, 0, 1);
					break;
				}
				offset = 0;
				ext_base = INVALID_CLUS;
				while (prt_num < 0 || num <= prt_num)
				{
					switch (bd->type)
					{
						case BDT_ATA:
							ATAReadBlks(bd, &mbr, offset, 1);
							break;
						case BDT_INT13H:
							Int13hTransferBlksFromBlkDev(bd, &mbr, offset, 1, FALSE);
							break;
					}
					if (mbr.signature != 0xAA55)
						break;
					j = -1;
					for (i = 0; i < 4 && (prt_num < 0 || num <= prt_num); i++)
					{
						if (mbr.p[i].type)
						{
							if (make_free)
								drive = DriveMakeFreeSlot(bd->first_drive_let + res);
							else
								drive = DriveMakeFreeSlot(DriveNextFreeLet(bd->first_drive_let + res));
							drive->bd			= bd;
							drive->drv_offset	= mbr.p[i].offset + offset;
							drive->size 		= mbr.p[i].size;
							switch (mbr.p[i].type)
							{
								case MBR_PT_ECHFS:
									drive->drive_signature = DRIVE_SIGNATURE_VAL;
									drive->prt_num = num;
									res++;
									drive->fs_type = FSt_ECHFS;
									EchFSInit(drive);
									break;
								case MBR_PT_REDSEA:
									drive->drive_signature = DRIVE_SIGNATURE_VAL;
									drive->prt_num = num;
									res++;
									drive->fs_type = FSt_REDSEA;
									RedSeaInit(drive);
									break;
								case MBR_PT_FAT32a:
								case MBR_PT_FAT32b:
								case MBR_PT_FAT32c:
								case MBR_PT_FAT32d:
								case MBR_PT_FAT32e:
								case MBR_PT_FAT32f:
									ATAReadBlks(bd, &br, drive->drv_offset, 1);
									drive->drive_signature = DRIVE_SIGNATURE_VAL;
									drive->prt_num = num;
									res++;
									if (br.signature == MBR_PT_REDSEA)
									{
										drive->fs_type = FSt_REDSEA;
										RedSeaInit(drive);
									}
									else
									{
										drive->fs_type = FSt_FAT32;
										FAT32Init(drive);
									}
									break;
								case MBR_PT_NTFS:
									drive->drive_signature = DRIVE_SIGNATURE_VAL;
									drive->prt_num = num;
									res++;
									drive->fs_type = FSt_NTFS;
									break;
								case 5:
								case 15:
									j = i;
									break;
								default:
									drive->drive_signature = DRIVE_SIGNATURE_VAL;
									drive->prt_num = num;
									res++;
									drive->fs_type = FSt_UNKNOWN;
							}
							num++;
						}
					}
					if (Letter2BlkDevType(bd->first_drive_let + res) != bd->type)
						break;
					if (j < 0)
						break;
					if (!mbr.p[j].offset)
						break;
					if (ext_base == INVALID_CLUS)
					{
						offset = mbr.p[j].offset;
						ext_base = offset;
					}
					else
						offset = mbr.p[j].offset + ext_base;
				}
			break;
		case BDT_ATAPI:
			drive->drive_signature = DRIVE_SIGNATURE_VAL;
			drive->prt_num = num;
			res++;
			drive->fs_type = FSt_ISO9660; //Start with this
			drive->size = 0;
			break;
	}
	if (res)
		BlkDevLockFwdingSet(bd);
	else
		BlkDevDel(bd);
	return res;
}

Bool DriveEnable(U8 drv_let, Bool val)
{//Can unmount or remount, but not mount the first time.
	CDrive *drive;
	if (drive = Letter2Drive(drv_let,FALSE))
		return !LBEqual(&drive->fs_type,FStf_DISABLE,!val);
	else
		return FALSE;
}

I64 SysI64Get()
{
	U8 st[STR_LEN];
	StrNGet(st, STR_LEN, FALSE);
	return Str2I64(st, 16);
}

Bool GetBaseUnit(CBlkDev *bd)
{
	I64 ch;
	Bool probe;
	#exe {
		if (kernel_config->opts[CONFIG_DONT_PROBE])
			StreamPrint("probe = FALSE;");
		else
			StreamPrint("probe = TRUE;");
	};
	if (!probe || !BootDVDProbeAll(bd))
	{
		"\nDon't worry.  This is not a product\n"
		"registration.				ZenithOS just needs the\n"
		"I/O port numbers for the CD/DVD.\n"
		"\nRetry the ports above or check Windows\n"
		"system information under I/O ports for\n"
		"'IDE', 'ATA' or 'SATA'.\n"
		"In Linux, use 'lspci -v' for ports.\n"
		"\n\nEnter 4-digit hex I/O Port number.\n"
		"CD/DVD I/O Port Base0: 0x";

		blkdev.dvd_boot_is_good = FALSE;

		while (!blkdev.dvd_boot_is_good)
		{
			bd->base0 = SysI64Get;
			bd->base1 = 0;
			bd->unit  = 0;
			if (bd->base0)
			{
				"\nUnit (0 or 1): ";
				do ch = CharGet(, FALSE);
				while (!('0' <= ch <= '1'));
				'' ch;
				bd->unit = ch - '0';
				'\n\n';
				blkdev.dvd_boot_is_good = BootDVDProbe(bd);
			}
		}
	}
	return TRUE;
}

U0 BlkDevsInitAll()
{
	CBlkDev *bd;
	I64 i;
	blkdev.blkdevs = CAlloc(sizeof(CBlkDev) * BLKDEVS_NUM);
	blkdev.drvs = CAlloc(sizeof(CDrive) * DRIVES_NUM);
	for (i = 0; i < DRIVES_NUM; i++)
		blkdev.let_to_drive[i] = &blkdev.drvs[i];
	#exe {
		if (kernel_config->opts[CONFIG_MOUNT_IDE_AUTO])
		{
			StreamPrint("MountIDEAuto;");
			StreamPrint("MountInt13hAuto;");
		}
		StreamPrint("#exe {Option(OPTf_WARN_PAREN, OFF);}");
		StreamDoc(kernel_config->add_dev);
		StreamPrint("#exe {Option(OPTf_WARN_PAREN, ON);}");
	};
}
