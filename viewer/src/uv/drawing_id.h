#ifndef UV_DRAWING_ID_H_
#define UV_DRAWING_ID_H_

namespace uv
{

typedef unsigned int DrawingID;

inline DrawingID get_free_drawing_id()
{
	static DrawingID drawing_id=1;
	return (drawing_id++);
}

inline bool drawing_id_valid(const DrawingID drawing_id)
{
	return (drawing_id>0);
}

}

#endif /* UV_DRAWING_ID_H_ */
