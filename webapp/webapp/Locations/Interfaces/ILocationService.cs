using System.Collections.Generic;
using System.Threading.Tasks;
using webapp.Locations.Models;

namespace webapp.Locations.Interfaces
{
    public interface ILocationService
    {
        Task<IEnumerable<LocationModel>> GetAllLocations();
        Task<LocationModel> AddLocation(LocationAddModel model);
    }
}
