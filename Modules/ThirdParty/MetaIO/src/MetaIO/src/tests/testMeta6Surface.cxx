#include <iostream>

#include <metaSurface.h>

int
main(int, char *[])
{
  std::cout << "Creating test file ...";
  auto * surface = new MetaSurface(3);
  surface->ID(0);
  SurfacePnt * pnt;

  unsigned int i;
  for (i = 0; i < 10; i++)
  {
    pnt = new SurfacePnt(3);
    pnt->m_X[0] = static_cast<float>(0.2);
    float i_f = static_cast<float>(i);
    pnt->m_X[1] = i_f;
    pnt->m_X[2] = i_f;
    pnt->m_V[0] = static_cast<float>(0.8);
    pnt->m_V[1] = i_f;
    pnt->m_V[2] = i_f;
    surface->GetPoints().push_back(pnt);
  }


  std::cout << "Writing ASCII test file ...";

  surface->Write("mySurface.meta");

  std::cout << "done" << '\n';
  std::cout << "Reading ASCII test file ...";

  surface->Clear();
  surface->Read("mySurface.meta");
  surface->PrintInfo();

  MetaSurface::PointListType                 list = surface->GetPoints();
  MetaSurface::PointListType::const_iterator it = list.begin();

  while (it != list.end())
  {
    for (unsigned int d = 0; d < 3; d++)
    {
      std::cout << (*it)->m_X[d] << " ";
    }
    std::cout << '\n';
    for (unsigned int d = 0; d < 3; d++)
    {
      std::cout << (*it)->m_V[d] << " ";
    }

    std::cout << '\n';
    for (float d : (*it)->m_Color)
    {
      std::cout << d << " ";
    }
    std::cout << '\n';
    ++it;
  }

  std::cout << "Writing Binary test file ...";
  surface->BinaryData(true);
  surface->ElementType(MET_FLOAT);
  surface->Write("mySurface.meta");

  std::cout << "done" << '\n';
  std::cout << "Reading Binary test file ...";

  surface->Clear();
  surface->Read("mySurface.meta");
  surface->PrintInfo();

  MetaSurface::PointListType list2 = surface->GetPoints();
  it = list2.begin();

  while (it != list2.end())
  {
    for (unsigned int d = 0; d < 3; d++)
    {
      std::cout << (*it)->m_X[d] << " ";
    }
    std::cout << '\n';
    for (unsigned int d = 0; d < 3; d++)
    {
      std::cout << (*it)->m_V[d] << " ";
    }
    std::cout << '\n';
    ++it;
  }

  delete surface;
  std::cout << "done" << '\n';
  return EXIT_SUCCESS;
}
