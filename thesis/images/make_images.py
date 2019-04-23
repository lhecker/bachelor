#!/usr/bin/env python3
import subprocess

import matplotlib.pyplot as plt
import numpy as np
import scipy.ndimage
import skimage.draw
import skimage.measure
import skimage.filters
import skimage.morphology
import skimage.transform


def pairwise(iterable):
    a = iter(iterable)
    return zip(a, a)


def binary_dilation(image, iterations=1):
    out = np.empty(image.shape, dtype=np.bool)
    scipy.ndimage.binary_dilation(image, output=out, iterations=iterations)
    return out


def write_image(name, image):
    plt.imsave(name, image, cmap='gray')
    try:
        subprocess.run([
            'pngout',
            '-y',
            name,
            name,
        ], check=True)
    except FileNotFoundError:
        pass


def essentials_binarization():
    image = skimage.data.page()
    write_image('essentials_binarization_uneven_illumination.png', image)
    binary = image >= skimage.filters.threshold_otsu(image)
    write_image('essentials_binarization_uneven_illumination_otsu.png', binary)
    binary = image >= skimage.filters.threshold_sauvola(image, window_size=21)
    write_image('essentials_binarization_uneven_illumination_sauvola.png', binary)

    image = skimage.data.camera()
    write_image('essentials_binarization_window_size.png', image)
    binary = image >= skimage.filters.threshold_otsu(image)
    write_image('essentials_binarization_window_size_otsu.png', binary)
    binary = image >= skimage.filters.threshold_sauvola(image, window_size=21)
    write_image('essentials_binarization_window_size_sauvola.png', binary)


def essentials_distance_transform():
    image = skimage.data.imread('holsteiner-stute.png', as_gray=True) > 0
    image_inv = skimage.util.invert(image)

    distance = scipy.ndimage.distance_transform_edt(image_inv)
    write_image('essentials_distance_transform_horse.png', skimage.util.invert(distance))


def essentials_skeletonize():
    image = skimage.data.imread('holsteiner-stute.png', as_gray=True) > 0
    image_inv = skimage.util.invert(image)

    distance = scipy.ndimage.distance_transform_edt(image_inv)
    distance = skimage.util.invert(distance.astype(np.uint8))
    distance_border = skimage.measure.find_contours(image, 0.5)

    bg: np.ndarray = skimage.img_as_ubyte(distance)
    bg_min, bg_max = np.amin(bg), np.amax(bg)
    bg_mid = np.ubyte((int(bg_min) + int(bg_max)) / 2)

    border: np.ndarray = np.zeros(bg.shape, dtype=np.bool_)
    for b in distance_border:
        for (x0, y0), (x1, y1) in pairwise(b):
            rr, cc = skimage.draw.line(int(x0), int(y0), int(x1), int(y1))
            border[rr, cc] = True

    def finish(name: str, skeleton: np.ndarray):
        skeleton = skimage.morphology.binary_dilation(skeleton)
        skeleton_border = skimage.morphology.binary_dilation(skeleton)

        out = bg.copy()
        out[skeleton_border & image_inv] = bg_min
        out[skeleton & image_inv] = 255
        out[border] = bg_mid

        write_image(name, out)

    finish('essentials_skeletonize_medial_axis.png', skimage.morphology.medial_axis(image_inv))
    finish('essentials_skeletonize_thinning.png', skimage.morphology.skeletonize(image_inv))


def theory_preprocess():
    image = skimage.data.imread('essentials_binarization_sauvola.png', as_gray=True) > 0
    image_inv = skimage.util.invert(image)

    skeleton = skimage.morphology.skeletonize(image_inv)
    write_image('theory_preprocess_skeleton.png', skimage.util.invert(skimage.morphology.binary_dilation(skeleton)))

    distance = scipy.ndimage.distance_transform_edt(image_inv)
    write_image('theory_preprocess_distance.png', skimage.util.invert(distance))

    redraw = np.full(image.shape, 255, dtype=np.uint8)
    with np.nditer([skeleton, distance], flags=['multi_index']) as it:
        while not it.finished:
            if it[0]:
                radius = int(it[1] + 0.5)
                rr, cc = skimage.draw.circle(it.multi_index[0], it.multi_index[1], radius, redraw.shape)
                redraw[rr, cc] = 0
            it.iternext()
    write_image('theory_preprocess_reconstructed.png', redraw)


essentials_binarization()
essentials_distance_transform()
essentials_skeletonize()
theory_preprocess()
