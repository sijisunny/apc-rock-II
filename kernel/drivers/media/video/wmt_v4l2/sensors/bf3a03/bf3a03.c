/*++
	linux/drivers/media/video/wmt_v4l2/sensors/bf3a03/bf3a03.c

	Copyright (c) 2013  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

#include "../cmos-subdev.h"
#include "../../wmt-vid.h"
#include "bf3a03.h"

#define sensor_write_array(sd, arr, sz) \
	cmos_init_8bit_addr(arr, sz, (sd)->i2c_addr)

#define sensor_read(sd, reg) \
	wmt_vid_i2c_read(sd->i2c_addr, reg)

#define sensor_write(sd, reg, val) \
	wmt_vid_i2c_write(sd->i2c_addr, reg, val)

struct cmos_win_size {
	char		*name;
	int		width;
	int		height;
	uint8_t 	*regs;
	size_t		size;
};

#define CMOS_WIN_SIZE(n, w, h, r) \
	{.name = n, .width = w , .height = h, .regs = r, .size = ARRAY_SIZE(r) }


static const struct cmos_win_size cmos_supported_win_sizes[] = {
//	CMOS_WIN_SIZE("QVGA", 320, 240, bf3a03_320x240),
	CMOS_WIN_SIZE("VGA",  640, 480, bf3a03_640x480),
};

static const struct cmos_win_size *cmos_select_win(u32 *width, u32 *height)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cmos_supported_win_sizes); i++) {
		if (cmos_supported_win_sizes[i].width == *width &&
		    cmos_supported_win_sizes[i].height == *height) {
			*width = cmos_supported_win_sizes[i].width;
			*height = cmos_supported_win_sizes[i].height;
			return &cmos_supported_win_sizes[i];
		}
	}
	return NULL;
}

static int sensor_s_wb(struct cmos_subdev *sd, enum v4l2_wb_mode value)
{
	uint8_t *regs;
	size_t size;

	switch (value) {
	case WHITE_BALANCE_AUTO:
		regs = bf3a03_wb_auto;
		size = ARRAY_SIZE(bf3a03_wb_auto);
		break;
	case WHITE_BALANCE_INCANDESCENCE:
		regs = bf3a03_wb_incandescent;
		size = ARRAY_SIZE(bf3a03_wb_incandescent);
		break;
	case WHITE_BALANCE_DAYLIGHT:
		regs = bf3a03_wb_daylight;
		size = ARRAY_SIZE(bf3a03_wb_daylight);
		break;
	case WHITE_BALANCE_CLOUDY:
		regs = bf3a03_wb_cloudy;
		size = ARRAY_SIZE(bf3a03_wb_cloudy);
		break;
	case WHITE_BALANCE_FLUORESCENT:
		regs = bf3a03_wb_fluorescent;
		size = ARRAY_SIZE(bf3a03_wb_fluorescent);
		break;
	default:
		return -EINVAL;
	}

	sensor_write_array(sd, regs, size);
	return 0;
}

static int sensor_s_scenemode(struct cmos_subdev *sd, enum v4l2_scene_mode value)
{
	uint8_t *regs;
	size_t size;

	switch (value) {
	case SCENE_MODE_AUTO:
		regs = bf3a03_scene_mode_auto;
		size = ARRAY_SIZE(bf3a03_scene_mode_auto);
		break;
	case SCENE_MODE_NIGHTSHOT:
		regs = bf3a03_scene_mode_night;
		size = ARRAY_SIZE(bf3a03_scene_mode_night);
		break;
	default:
		return -EINVAL;
	}

	sensor_write_array(sd, regs, size);
	return 0;
}

static int sensor_s_exposure(struct cmos_subdev *sd, int value)
{
	uint8_t *regs;
	size_t size;

	switch (value) {
	case -2:
		regs = bf3a03_exposure_neg6;
		size = ARRAY_SIZE(bf3a03_exposure_neg6);
		break;
	case -1:
		regs = bf3a03_exposure_neg3;
		size = ARRAY_SIZE(bf3a03_exposure_neg3);
		break;
	case 0:
		regs = bf3a03_exposure_zero;
		size = ARRAY_SIZE(bf3a03_exposure_zero);
		break;
	case 1:
		regs = bf3a03_exposure_pos3;
		size = ARRAY_SIZE(bf3a03_exposure_pos3);
		break;
	case 2:
		regs = bf3a03_exposure_pos6;
		size = ARRAY_SIZE(bf3a03_exposure_pos6);
		break;
	default:
		return -EINVAL;
	}

	sensor_write_array(sd, regs, size);
	return 0;
}

static int sensor_s_hflip(struct cmos_subdev *sd, int value)
{
	uint8_t data;
	uint8_t retry_times=0;
	
	 
	data = sensor_read(sd, 0x1e);

	switch (value) {
	case 0:
		data &= ~0x20;
		break;
	case 1:
		data |= 0x20;
		break;
	default:
		return -EINVAL;
	}
	
	sensor_write(sd, 0x1e, data);
	return 0;
}

static int sensor_s_vflip(struct cmos_subdev *sd, int value)
{
	uint8_t data;
	uint8_t retry_times=0;

	 

	data = sensor_read(sd, 0x1e);

	switch (value) {
	case 0:
		data &= ~0x10;
		break;
	case 1:
		data |= 0x10;
		break;
	default:
		return -EINVAL;
	}
	sensor_write(sd, 0x1e, data);


	return 0;
}

static int sensor_queryctrl(struct cmos_subdev *sd, struct v4l2_queryctrl *qc)
{
	switch (qc->id) {
	case V4L2_CID_VFLIP:
	case V4L2_CID_HFLIP:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
	case V4L2_CID_CAMERA_SCENE_MODE:
		return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);
	case V4L2_CID_DO_WHITE_BALANCE:
		return v4l2_ctrl_query_fill(qc, 0, 3, 1, 0);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, -2, 2, 1, 0);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct cmos_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_CAMERA_SCENE_MODE:
		return sensor_s_scenemode(sd, ctrl->value);
	case V4L2_CID_DO_WHITE_BALANCE:	
		return sensor_s_wb(sd, ctrl->value);	
	case V4L2_CID_EXPOSURE:
		return sensor_s_exposure(sd, ctrl->value);	
	case V4L2_CID_HFLIP:
		return sensor_s_hflip(sd, ctrl->value);
	case V4L2_CID_VFLIP:
		return sensor_s_vflip(sd, ctrl->value);
	default:
	case WMT_V4L2_CID_CAMERA_ANTIBANDING:
		return -EINVAL;
	}
	return -EINVAL;
}

static int sensor_g_mbus_fmt(struct cmos_subdev *sd,
			     struct v4l2_mbus_framefmt *mf)
{
	return -EINVAL;
}

static int sensor_s_mbus_fmt(struct cmos_subdev *sd,
			     struct v4l2_mbus_framefmt *mf)
{
	const struct cmos_win_size *win;

	win = cmos_select_win(&mf->width, &mf->height);
	if (!win) {
		pr_err("%s, s_mbus_fmt failed, width %d, height %d\n", 
		       sd->name, mf->width, mf->height);
		return -EINVAL;
	}

	sensor_write_array(sd, win->regs, win->size);
	return 0;
}

static int sensor_try_mbus_fmt(struct cmos_subdev *sd,
			       struct v4l2_mbus_framefmt *mf)
{
	return 0;
}

static int sensor_enum_framesizes(struct cmos_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;

	for (i = 0; i < ARRAY_SIZE(cmos_supported_win_sizes); i++) {
		const struct cmos_win_size *win = &cmos_supported_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int sensor_identify(struct cmos_subdev *sd)
{
	uint32_t data = 0;

	data |= (sensor_read(sd, 0xfc) & 0xff) << 8;
	data |= (sensor_read(sd, 0xfd) & 0xff);
		
	return (data == sd->id) ? 0 : -EINVAL;
}

static int sensor_init(struct cmos_subdev *sd)
{
	uint32_t data = 0;
	
	if (sensor_identify(sd)) {
		return -1;
	}

	sensor_write_array(sd, bf3a03_default_regs_init,
			   ARRAY_SIZE(bf3a03_default_regs_init));
	return 0;
}

static int sensor_exit(struct cmos_subdev *sd)
{
	sensor_write_array(sd, bf3a03_default_regs_exit,
			   ARRAY_SIZE(bf3a03_default_regs_exit));
	return 0;
}

static struct cmos_subdev_ops bf3a03_ops = {
	.identify	= sensor_identify,
	.init		= sensor_init,
	.exit		= sensor_exit,
	.queryctrl	= sensor_queryctrl,
	.s_ctrl		= sensor_s_ctrl,
	.s_mbus_fmt     = sensor_s_mbus_fmt,
	.g_mbus_fmt     = sensor_g_mbus_fmt,
	.try_mbus_fmt	= sensor_try_mbus_fmt,
	.enum_framesizes = sensor_enum_framesizes,
};

struct cmos_subdev bf3a03 = {
	.name		= "bf3a03",
	.i2c_addr	= 0x6e,
	.id		= 0x3a03,
	.max_width	= 640,
	.max_height	= 480,
	.ops		= &bf3a03_ops,
};

#if 0
static int __init bf3a03_init(void)
{
	return cmos_register_sudbdev(&bf3a03);
}

static void __exit bf3a03_exit(void)
{
	return cmos_unregister_subdev(&bf3a03);
}

module_init(bf3a03_init);
module_exit(bf3a03_exit);

MODULE_LICENSE("GPL");
#endif
