#file(GLOB hdrs "*.h*") #Добавляем в переменную hdrs все файлы с расширением .h
file(GLOB srcs "*.cpp") #Добавляем в переменную srcs все файлы с расширением .h

add_executable(${main} ${srcs} ${hdrs})
target_link_libraries(${main} ${queue_tasks})
target_link_libraries(${main} ${MPI_LIBRARIES})
