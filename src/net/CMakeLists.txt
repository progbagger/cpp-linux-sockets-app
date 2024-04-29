add_library(net STATIC 
  address.cc
  socket.cc
  server.cc
  client.cc

  ${CMAKE_SOURCE_DIR}/include/net/address.h
  ${CMAKE_SOURCE_DIR}/include/net/socket.h
  ${CMAKE_SOURCE_DIR}/include/net/server.h
  ${CMAKE_SOURCE_DIR}/include/net/client.h
)
target_include_directories(net PUBLIC ${CMAKE_SOURCE_DIR})
