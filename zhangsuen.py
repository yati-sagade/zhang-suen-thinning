import sys
import cv2
from cv2 import cv
import numpy as np


def display(windowname, image):
    cv.NamedWindow(windowname, cv.CV_WINDOW_NORMAL)
    cv2.imshow(windowname, image)


def connectivity(image, point):
    r, c = point
    return sum((image[r][c+1] >= 1 and image[r-1][c+1] == 0,
                image[r-1][c+1] >= 1 and image[r-1][c] == 0,
                image[r-1][c] >= 1 and image[r-1][c-1] == 0,
                image[r-1][c-1] >= 1 and image[r][c-1] == 0,
                image[r][c-1] >= 1 and image[r+1][c-1] == 0,
                image[r+1][c-1] >= 1 and image[r+1][c] == 0,
                image[r+1][c] >= 1 and image[r+1][c+1] == 0,
                image[r+1][c+1] >= 1 and image[r][c+1] == 0))


def num_one_neighbours(image, point):
    r, c = point
    count = 0
    for pt in ((r, c+1), (r-1, c+1), (r-1, c), (r-1, c-1), 
               (r, c-1), (r+1, c-1), (r+1, c), (r+1, c+1)):
        if image[pt] >= 1:
            count += 1
    return count


def is_deletion_candidate(image, point, subiteration):
    r, c = point
    pixel = image[point]
    if (pixel >= 1 and
        connectivity(image, point) == 1 and
        2 <= num_one_neighbours(image, point) <= 6):
        if subiteration == 0:
            if (image[r, c+1] * image[r-1, c] * image[r, c-1] == 0 and
                image[r-1, c] * image[r+1, c] * image[r, c-1] == 0):
                return True
        elif subiteration == 1:
            if (image[r-1, c] * image[r, c+1] * image[r+1, c] == 0 and
                image[r, c+1] * image[r+1, c] * image[r, c-1] == 0):
                return True
        else:
            raise ValueError('subiteration can only be 0 or 1')
    return False


def thin(image):
    zeros, nonzeros = image == 0, image != 0
    image[nonzeros] = 0
    image[zeros] = 1
    again = True
    tmp = np.zeros(image.shape, dtype=np.bool)
    iterations = 1
    while again:
        again = False
        print('Iteration {}'.format(iterations))
        iterations += 1
        for i in xrange(1, image.shape[0] - 1):
            for j in xrange(1, image.shape[1] - 1):
                if is_deletion_candidate(image, (i, j), 0):
                    tmp[i, j] = True
                    again = True
        image[tmp] = 0
        if not again:
            break
        print('{} flagged in sub-iteration 0'.format(np.count_nonzero(tmp)))
        for i in xrange(1, image.shape[0] - 1):
            for j in xrange(1, image.shape[1] - 1):
                if is_deletion_candidate(image, (i, j), 1):
                    tmp[i, j] = True
                    again = True
        image[tmp] = 0
        print('{} flagged in sub-iteration 0'.format(np.count_nonzero(tmp)))
    zeros, nonzeros = image == 0, image != 0
    image[nonzeros] = 0
    image[zeros] = 255


def main():
    image = cv2.imread(sys.argv[1], 0)
    dst = np.copy(image)
    thin(dst)
    display('Original', image)
    display('Thinned', dst)
    cv.WaitKey(0)


if __name__ == '__main__':
    main()
